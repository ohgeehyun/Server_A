#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Monster.h"
#include "Arrow.h"
#include "MagicSkill.h"
#include "RoomManager.h"
#include "RedisConnection.h"

Room::Room()
{
    
}

Room::~Room()
{
}

void Room::Init(int32 mapId)
{
    _map.LoadMap(mapId);
}

void Room::Update()
{
    for (const auto& pair : _projectTiles)
    {
        pair.second->Update();
    }
    for (const auto& pair : _monsters)
    {
        pair.second->Update();
    }
}

PlayerRef Room::FindPlayer(std::function<bool(const GameObjectRef&)> condition)
{
    for (const auto& pair : _players)
    {
        if (std::invoke(condition, pair.second))
            return pair.second;
    }

    return nullptr;
}


void Room::EnterGame(GameObjectRef gameObject)
{
    if (gameObject == nullptr)
        return;
    
    Protocol::GameObjectType type = ObjectManager::GetObjectTypeById(gameObject->GetObjectId());
    
    switch (type)
    {
    case Protocol::PLAYER: 
        {
            DoAsync(&Room::EnterGame_Player, dynamic_pointer_cast<Player>(gameObject));
        }
        break;
        case Protocol::MONSTER: 
        {
             DoAsync(&Room::EnterGame_Monster, dynamic_pointer_cast<Monster>(gameObject));
        }
        break;
        case Protocol::PROJECTTILE:
        {
            DoAsync(&Room::EnterGame_ProjectTile, dynamic_pointer_cast<ProjectTile>(gameObject));
        }
        break;
        case Protocol::MAGIC :
        {
            DoAsync(&Room::EnterGame_ProjectTile, dynamic_pointer_cast<ProjectTile>(gameObject));
        }
        break;
    }
}

void Room::EnterGame_Player(PlayerRef player)
{
    if (player == nullptr)
        return;

    if (tempSpawnHandle == false)
    {
        DoAsync(std::bind(&Room::SpawnMonster,this,5 ,5));
        DoAsync(std::bind(&Room::SpawnMonster, this, 8, 3));
        DoAsync(std::bind(&Room::SpawnMonster, this, 10, 7));
        tempSpawnHandle = true;
    }

    DoAsync([this, player]() {
        _players.insert(make_pair(player->GetObjectId(), player));
        player->SetRoom(GetSharedRoomPtr());
        _map.ApplyMove(static_pointer_cast<GameObject>(player),Vector2Int(player->GetPosx(),player->GetPosy()));
        EnterGameEventSend_Player(player);
    });
}

void Room::EnterGame_Monster(MonsterRef monster)
{
    DoAsync([this,monster](){
        _monsters.insert(std::make_pair(monster->GetObjectId(), monster));
        monster->SetRoom(GetSharedRoomPtr());
        _map.ApplyMove(static_pointer_cast<GameObject>(monster), Vector2Int(monster->GetPosx(), monster->GetPosy()));
        EnterGameEventSend_Monster(monster);
    });
    
}

void Room::EnterGame_ProjectTile(ProjectTileRef projectTile)
{
    DoAsync([this, projectTile]() {
        _projectTiles.insert(make_pair(projectTile->GetObjectId(), projectTile));
        projectTile->SetRoom(GetSharedRoomPtr());
        EnterGameEventSend_ProjectTile(projectTile);
    });
}

void Room::EnterGameEventSend_Player(PlayerRef player)
{
    if (player == nullptr)
        return;

    cout << player->GetObjectId() << endl;
    Protocol::S_ENTER_GAME enterPacket;
    Protocol::OBJECT_INFO playerInfo = player->GetObjectInfo();
    enterPacket.set_roomid(_roomId);
    enterPacket.set_roomname(_roomName);
    *enterPacket.mutable_player() = playerInfo;

    auto enterPacketBuffer = ClientPacketHandler::MakeSendBuffer(enterPacket);
    player->GetSession()->Send(enterPacketBuffer);

    //룸에 존재하는 인원들 정보를 새로 접속한 인원에게 전송
    {
        Protocol::S_SPAWN SpawnPacket;
        for (const auto& playerPair : _players)
        {
            const PlayerRef& p = playerPair.second;
            if (p != player)
            {
                *SpawnPacket.add_objects() = p->GetObjectInfo();
            }
        }

        for (const auto& monsterPair : _monsters)
        {
            const MonsterRef& m = monsterPair.second;
            *SpawnPacket.add_objects() = m->GetObjectInfo();
        }

        for (const auto& tilepair : _projectTiles)
        {
            const ProjectTileRef& p = tilepair.second;
            *SpawnPacket.add_objects() = p->GetObjectInfo();
        }

        auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
        player->GetSession()->Send(SpawnPacketBuffer);
    }

    //이미 룸에 존재하던 인원에게 새로운 인원 정보 전송
    {
        Protocol::S_SPAWN SpawnPacket;
        SpawnPacket.add_objects()->CopyFrom(player->GetObjectInfo());

        auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
        for (const auto& playerPair : _players)
        {
            if (player->GetObjectId() != playerPair.second->GetObjectId())
                playerPair.second->GetSession()->Send(SpawnPacketBuffer);
        }
    }

    //현재는 방을 떠날때 Redis에서도 삭제 시켜주지만 나중에 방을나가는 기능 추가시 거기로 이동
    redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
    {
        if (reply != nullptr)
            RedisUtils::replyResponseHandler(reply, "Redis Room join user : ");

    }, nullptr, "SADD room_user:%d %s",_roomId , player->GetSession()->GetUserId());
}

void Room::EnterGameEventSend_Monster(MonsterRef monster)
{
    if (monster == nullptr)
        return;

    Protocol::S_SPAWN SpawnPacket;
    SpawnPacket.add_objects()->CopyFrom(monster->GetObjectInfo());
    auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
    DoAsync(&Room::Broadcast,SpawnPacketBuffer);
}

void Room::EnterGameEventSend_ProjectTile(ProjectTileRef projecTtile)
{
    if (projecTtile == nullptr)
        return;

    Protocol::S_SPAWN SpawnPacket;
    SpawnPacket.add_objects()->CopyFrom(projecTtile->GetObjectInfo());
    auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
    DoAsync(&Room::Broadcast, SpawnPacketBuffer);
}

void Room::LeaveGame(int32 objectId)
{
    if (objectId < 0)
        return;

    Protocol::GameObjectType type = ObjectManager::GetObjectTypeById(objectId);

    switch (type)
    {
        case Protocol::PLAYER:
            DoAsync(&Room::LeaveGame_Player,objectId);
            break;
        case Protocol::MONSTER:
            DoAsync(&Room::LeaveGame_Monster, objectId);
            break;
        case Protocol::PROJECTTILE:
            DoAsync(&Room::LeaveGame_ProjectTile, objectId);
            break;
        case Protocol::MAGIC:
            DoAsync(&Room::LeaveGame_ProjectTile, objectId);
            break;
    }
}

void Room::LeaveGame_Player(int32 objectId)
{
    auto it = std::find_if(_players.begin(), _players.end(), [objectId](const std::pair<const int32, PlayerRef>& pair) {
        return pair.second->GetObjectId() == objectId;
    });

    if (it == _players.end())
        return;

    DoAsync([this,it]() {
        _map.ApplyLeave(static_pointer_cast<GameObject>(it->second));

        LeaveGameEventSend_Player(it->second, it->second->GetObjectId());

        //플레이어 와 Room 의존 끊기
        it->second->SetRoom(nullptr);

        //플레이어 목록에서 삭제
        _players.erase(it);
    });

    redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
    {
        if (reply != nullptr)
            RedisUtils::replyResponseHandler(reply, "Redis Room Exit user : ");

    }, nullptr, "SADD room_user:%d %s", _roomId, it->second->GetSession()->GetUserId());
}

void Room::LeaveGame_Monster(int32 objectId)
{
    auto it = std::find_if(_monsters.begin(), _monsters.end(), [objectId](const std::pair<const int32, MonsterRef>& pair) {
        return pair.second->GetObjectInfo().objectid() == objectId;
    });

    if (it == _monsters.end())
        return;

    DoAsync([this, it]() {
        _map.ApplyLeave(static_pointer_cast<Monster>(it->second));

        LeaveGameEventSend_Monster(it->second, it->second->GetObjectId());

        it->second->SetRoom(nullptr);

        _monsters.erase(it);
    });
}

void Room::LeaveGame_ProjectTile(int32 objectId)
{
    auto it = std::find_if(_projectTiles.begin(), _projectTiles.end(), [objectId](const std::pair<const int32, ProjectTileRef>& pair) {
        return pair.second->GetObjectInfo().objectid() == objectId;
    });

    if (it == _projectTiles.end())
        return;

    DoAsync([this, it]() {

        ProjectTileRef projecttile = static_pointer_cast<ProjectTile>(it->second);
        _map.ApplyLeave(projecttile);

        LeaveGameEventSend_ProjectTile(it->second, it->second->GetObjectId());

        it->second->SetRoom(nullptr);

        _projectTiles.erase(it);

        projecttile->SetOwner(nullptr);
    });
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
    if (_players.size() == 0)
        return;

    for (const auto& playerPair : _players)
    {
        playerPair.second->GetSession()->Send(sendBuffer);
        cout << "Broadcast 전송된 플레이어 id" << playerPair.second->GetObjectInfo().objectid() << endl;
    }
}

void Room::BroadcastExcept(SendBufferRef sendBuffer, PlayerRef player)
{
    if (_players.size() == 0)
        return;

    for (const auto& playerPair : _players)
    {
        if (playerPair.second != player)
        {
            playerPair.second->GetSession()->Send(sendBuffer);
            cout << "Broadcast 전송된 플레이어 id" << playerPair.second->GetObjectInfo().objectid() << endl;
        }
    }
}

void Room::SpawnMonster(int32 y, int32 x)
{
    // TEMP 몬스터 소환
    MonsterRef monster = ObjectManager::GetInstance().Add<Monster>();
    monster->SetCellPos(x, y);

    GameObjectRef gameObject = static_pointer_cast<GameObject>(monster);
    DoAsync(&Room::EnterGame,gameObject);
}

void Room::LeaveGameEventSend_Player(PlayerRef player,int32 objectid)
{
    //본인에게 퇴장 패킷 전송
    Protocol::S_LEAVE_GAME leavePacket;
    leavePacket.set_exit(true);
    auto leavePacketBuffer = ClientPacketHandler::MakeSendBuffer(leavePacket);
    player->GetSession()->Send(leavePacketBuffer);

    //서버에 접속인원에게 퇴장 패킷 생성
    Protocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
        if (objectid != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }
}

void Room::LeaveGameEventSend_Monster(MonsterRef monster, int32 objectid)
{
    //서버에 접속인원에게 퇴장 패킷 생성
    Protocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
        if (objectid != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }
}

void Room::LeaveGameEventSend_ProjectTile(ProjectTileRef projectTile, int32 objectid)
{
    //서버에 접속인원에게 퇴장 패킷 생성
    Protocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
        if (objectid != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }
}

void Room::HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt)
{
    if (player == nullptr)
        return;
    
    if (pkt.posinfo().posx() != player->GetPosx() || pkt.posinfo().posy() != player->GetPosy())
    {
        if (_map.CanGo(Vector2Int(pkt.posinfo().posx(), pkt.posinfo().posy())) == false)
            return;
    }
    
    DoAsync([this, player, pkt]() {
        player->SetState(pkt.posinfo().state());
        player->SetMoveDir(pkt.posinfo().movedir());
        _map.ApplyMove(static_pointer_cast<GameObject>(player), Vector2Int(pkt.posinfo().posx(), pkt.posinfo().posy()));
        HandleMoveEvent(player, pkt);
    });

  
}

void Room::HandleMoveEvent(PlayerRef player, Protocol::C_MOVE pkt)
{
    //다른 플레이어에게도 이동 전달
    Protocol::S_MOVE resMovePacket;
    resMovePacket.set_objectid(player->GetObjectInfo().objectid());
    resMovePacket.mutable_posinfo()->set_movedir(pkt.posinfo().movedir());
    resMovePacket.mutable_posinfo()->set_posx(pkt.posinfo().posx());
    resMovePacket.mutable_posinfo()->set_posy(pkt.posinfo().posy());
    resMovePacket.mutable_posinfo()->set_state(pkt.posinfo().state());

    auto resMovePacketBuffer = ClientPacketHandler::MakeSendBuffer(resMovePacket);
    for (const auto& pair : _players)
    {
        if (player != pair.second)
        {
            pair.second->GetSession()->Send(resMovePacketBuffer);
            cout << "이동 이벤트 전송 받은 플레이어 id" << pair.second->GetObjectInfo().objectid() << endl;
        }
    }
}

void Room::HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt)
{
    if (player == nullptr)
        return;

    //외부에서 다른 곳에서  playerinfo가수정이될수도있기때문에 미리 정보를 받아둠
    Protocol::OBJECT_INFO info = player->GetObjectInfo();
    if (info.posinfo().state() != Protocol::CreatureState::IDLE)
        return;

    // TODO : 스킬 사용 가능 여부 체크

   //스킬 사용 애니메이션을 플레이어들에게 전송
    info.mutable_posinfo()->set_state(Protocol::CreatureState::SKILL);

    Protocol::S_SKILL skill;

    skill.set_objectid(info.objectid());
    skill.mutable_info()->set_skillid(pkt.info().skillid());
    auto resSkillPacketBuffer = ClientPacketHandler::MakeSendBuffer(skill);
    Broadcast(resSkillPacketBuffer);

   
   /*
    반복문으로 skill검색 unorderdedmap의 내장 find함수 사용하여 주석처리 필요시 해제해서 사용
    auto it = std::find_if(DataManager::GetInstance().GetSkillDict().begin(), DataManager::GetInstance().GetSkillDict().end(),
        [pkt](const std::pair<const int32, Skill>& pair) {
        return pair.first == pkt.info().skillid();
    });*/

    auto it = DataManager::GetInstance().GetSkillDict().find(pkt.info().skillid());

    if (it == DataManager::GetInstance().GetSkillDict().end())
        return;

    Skill skillData = it->second;

    switch (skillData.skillType)
    {
    case Protocol::SKILL_AUTO:
    {
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
        GameObjectRef target = _map.Find(skillPos);
        if (target != nullptr)
        {
            cout << "Hit GameObject !" << endl;
        }
    }
    break;
    case Protocol::SKILL_PROJECTILE:
    {
        ArrowRef arrow = ObjectManager::GetInstance().Add<Arrow>();
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
        if (arrow == nullptr)
            return;

        //posinfo가아닌 object의 info의 posinfo에서 봐야함  
        arrow->SetOwner(player);
        arrow->SetSkillData(skillData);
        arrow->GetObjectInfo().mutable_posinfo()->set_state(Protocol::MOVING);
        arrow->GetObjectInfo().mutable_posinfo()->set_movedir(player->GetMoveDir());
        arrow->GetObjectInfo().mutable_posinfo()->set_posx(skillPos.posx);
        arrow->GetObjectInfo().mutable_posinfo()->set_posy(skillPos.posy);
        arrow->SetSpeed(skillData.projectile.speed);

        GameObjectRef gameObject = static_pointer_cast<GameObject>(arrow);
        DoAsync(&Room::EnterGame, gameObject);
    }
    break;
    case Protocol::SKILL_MAGIC:
    {
        MagicSkillRef magic = ObjectManager::GetInstance().Add<MagicSkill>();
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());

        if (magic == nullptr)
            return;

        magic->SetOwner(player);
        magic->SetSkillData(skillData);
        //magic->GetObjectInfo().mutable_posinfo()->set_state(Protocol::MOVING);
        //magic->GetObjectInfo().mutable_posinfo()->set_movedir(player->GetMoveDir());
        magic->GetObjectInfo().mutable_posinfo()->set_posx(skillPos.posx);
        magic->GetObjectInfo().mutable_posinfo()->set_posy(skillPos.posy);
        magic->SetSpeed(skillData.projectile.speed);

        GameObjectRef gameObject = static_pointer_cast<GameObject>(magic);
        DoAsync(&Room::EnterGame, gameObject);
    }
    break;
    }

}

