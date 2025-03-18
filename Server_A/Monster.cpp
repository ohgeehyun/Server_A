#include "pch.h"
#include "Monster.h"
#include "Protocol.pb.h"
#include "GameObject.h"
#include "Player.h"
#include "Room.h"
#include "ClientPacketHandler.h"
#include "DataContent.h"
#include "DataManager.h"
#include "RedisConnection.h"
#include "GameSession.h"


Monster::Monster()
{
    SetGameObjectType(Protocol::MONSTER);

    SetLevel(1);
    SetHp(100);
    SetMaxHp(100);
    SetSpeed(5.0f);
    SetState(Protocol::CreatureState::IDLE);
}

Monster::~Monster()
{
}

void Monster::Update()
{
    switch (GetState())
    {
    case Protocol::CreatureState::IDLE:
        UpdateIdle();
        break;
    case Protocol::CreatureState::MOVING:
        UpdateMoving();
        break;
    case Protocol::CreatureState::SKILL:
        UpdateSkill();
        break;
    case Protocol::CreatureState::DEAD:
        UpdateDead();
        break;
    }
}


void Monster::BroadCastMove()
{
    //다른 플레이어에게 몬스터의 움직임을 전송
    Protocol::S_MOVE _movePacket;
    _movePacket.set_objectid(GetObjectId());
    _movePacket.mutable_posinfo()->CopyFrom(GetPosinfo());

    auto movePacket = ClientPacketHandler::MakeSendBuffer(_movePacket);
    GetRoom()->DoAsync(&Room::Broadcast, movePacket);
}

void Monster::OnDead(GameObjectRef attacker)
{
    //캐릭터가 죽고 방에서 leave enter를 비동기적으로 처리 방식이라 객체와 room이 끊길수있어서 미리 변수로만들어서 사용
    RoomRef room = GetRoom();
    PlayerRef player = static_pointer_cast<Player>(attacker);

    _target = nullptr;

    GameObject::OnDead(attacker);

    redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
    {
        if (reply != nullptr)
            RedisUtils::replyResponseHandler(reply, "kill Score Add : ");

    }, nullptr, "HINCRBY room_score:%d:%s kill 1",room->GetRoomId(), player->GetSession()->GetUserId().c_str());
    cout << player->GetSession()->GetUserId() << endl;
}

void Monster::OnDameged(GameObjectRef attacker, int32 damege)
{
    PlayerRef player = static_pointer_cast<Player>(attacker);
    RoomRef room = GetRoom();

    GameObject::OnDameged(attacker, damege);

    if (attacker->GetGameObjectType() == Protocol::PLAYER && attacker->GetObjectId() != GetObjectId())
    {
        redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
        {
            if (reply != nullptr)
                RedisUtils::replyResponseHandler(reply, "damege Score Add : ");

        }, nullptr, "HINCRBY room_score:%d:%s TotalDamege %d", room->GetRoomId(), player->GetSession()->GetUserId().c_str(), damege);
    }
}

void Monster::UpdateIdle()
{
    if (_nextSearchTick > GetTickCount64())
        return;

    _nextSearchTick = GetTickCount64() + 1000;
    //1초마다 진입 효율적인 방식은아님. 해당 함수를 들어올떄마다 if문을 하게된다는 뜻.

    int32 cellDist = GetSearchCellDist();
    PlayerRef target = GetRoom()->FindPlayer([this,cellDist](const GameObjectRef& p) -> bool {
        Vector2Int dir = GetCellPos() - p->GetCellPos();
        return dir.cellDistFromZero() <= cellDist;
    });

    if (target == nullptr)
        return;

    _target = target;
    SetState(Protocol::CreatureState::MOVING);
}

void Monster::UpdateMoving()
{
    if (_nextSearchTick > GetTickCount64())
        return;

    int32 moveTick = static_cast<int32>(1000 / GetSpeed());
    _nextSearchTick = GetTickCount64() + moveTick;


    if (_target == nullptr || _target->GetRoom() != GetRoom())
    {
        _target = nullptr;
        SetState(Protocol::CreatureState::IDLE);
        BroadCastMove();
        return;
    }

    Vector2Int dir = _target->GetCellPos() - GetCellPos();
    int dist = dir.cellDistFromZero();
    if (dist == 0 || dist > GetChaseCellDist())
    {
        _target = nullptr;
        SetState(Protocol::CreatureState::IDLE);
        BroadCastMove();
        return;
    }

    Vector<Vector2Int> path = GetRoom()->GetMap().FindPath(GetCellPos(), _target->GetCellPos(), true);
    if (path.size() < 2 || path.size() > GetChaseCellDist())
    {
        _target = nullptr;
        SetState(Protocol::CreatureState::IDLE);
        BroadCastMove();
        return;
    }

    // 이동
    SetMoveDir(GetDirFromVec(path[1] - GetCellPos()));
    GetRoom()->DoAsync([this, path]() {
        GetRoom()->GetMap().ApplyMove(shared_from_this(), path[1]);
        //GetRoom()->DoAsync(std::bind(&MapManager::ApplyMove, GetRoom()->GetMap(), shared_from_this(), path[1]));
    });

    BroadCastMove();

    //스킬로 넘어갈지 체크
    if (dist <= _skillRange && (dir.posx == 0 || dir.posy == 0))
    {
        _coolTick = 0;
        SetState(Protocol::CreatureState::SKILL);
        return;
    }
}




void Monster::UpdateSkill()
{
    if (_coolTick == 0)
    {
        //유효한 타겟인지 
        if (_target == nullptr || _target->GetRoom() != GetRoom() || _target->GetHp() <= 0)
        {
            _target = nullptr;
            SetState(Protocol::CreatureState::MOVING);
            BroadCastMove();
            return;
        }

        //스킬이 아직 사용 가능한지
        Vector2Int dir = (_target->GetCellPos() - GetCellPos());
        int dist = dir.cellDistFromZero();
        bool canUseSkill = (dist <= _skillRange && (dir.posx == 0 || dir.posy == 0));
        if (canUseSkill == false)
        {
            SetState(Protocol::CreatureState::MOVING);
            BroadCastMove();
            return;
        }

        //타켓팅 방향 주시
        Protocol::MoveDir lookDir = GetDirFromVec(dir);
        if (GetMoveDir() != lookDir)
        {
            SetMoveDir(lookDir);
            BroadCastMove();
        }

        Skill skillData;
        for (auto& pair : DataManager::GetInstance().GetSkillDict())
        {
            // TODO : 지금은 수동으로 1이라는  데이터를 입력하였지만 나중에는 데이터로 뺴놓을 필요가있다.
            if (pair.first == 1)
                skillData = pair.second;
        }

        //데미지 판정
        GetRoom()->DoAsync([this, skillData]() {
            _target->OnDameged(shared_from_this(), skillData.damege + GetObjectStat().attack());
        });
       
        //스킬 사용 BroadCast
        Protocol::S_SKILL skill;
        skill.set_objectid(GetObjectId());
        skill.mutable_info()->set_skillid(skillData.id);
        auto skillPacket = ClientPacketHandler::MakeSendBuffer(skill);
        GetRoom()->DoAsync(&Room::Broadcast, skillPacket);

        //스킬 쿨타임 적용
        int coolTick = (int)(1000 * skillData.cooldown);
        _coolTick = GetTickCount64() + coolTick;
    }
    // 시간 체크를  사용하여 유니티의 코루틴을 따라하였는데 좋은 방법이 생기면 수정 필요
    if (_coolTick > GetTickCount64())
        return;

    _coolTick = 0;
}

void Monster::UpdateDead()
{
}
