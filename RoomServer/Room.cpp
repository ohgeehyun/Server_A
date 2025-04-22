#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "ServerProtocol.pb.h"
#include "RoomPacketHandler.h"
#include "UserServerSession.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "Monster.h"
#include "Arrow.h"
#include "MagicSkill.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include <httplib/httplib.h> 

Room::Room() 
{
    
}

Room::~Room()
{

    const char* query = "DEL room:%d";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _roomId);

    query = "DEL room_user:%d";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _roomId);

    cout << _roomId << " 번 방 소멸자 호출 완료. Redis 삭제 호출 완료" << endl;
}

void Room::Init(int32 mapId)
{
    _map.LoadMap(mapId);
}

void Room::RoomBreak(PlayerRef player)
{
    ServerProtocol::S_EXIT_GAME packet;
    packet.set_exitflag(true);
    auto exitPacketBuffer = RoomPacketHandler::MakeSendBuffer(packet);
    BroadcastExcept(exitPacketBuffer, player);

    HashMap<string, ServerConfigData> dict = DataManager::GetInstance().GetServerConfigDict();

    std::string host = "http://" + dict["database"].nodeData.host + ":" + dict["database"].nodeData.port;

    httplib::Client cli(host);
    httplib::Params params;

    params.emplace("roomId", std::to_string(_roomId));

    if (auto res = cli.Post("/chat/save", params))
    {
        if (res->status == 201) //restful에서 특정 데이터 추가 완료시 201 반환
            std::cout << _roomId << " 번 방 채팅 로그 저장 호출 완료" << "\n";
        else
            std::cout <<"error : HTTP Status" << _roomId << " 번 방 채팅 로그 저장 호출 중 문제 발생" << "\n";
    }
    else
    {
        std::cout << "http Request failed:" << "\n";
    }


    for (auto& player : _players)
    {
        player.second->SetRoom(nullptr);
    }
    _players.clear();

    for (auto& monster : _monsters)
    {
        monster.second->SetRoom(nullptr);
    }

    _monsters.clear();

    for (auto& projectTile : _projectTiles)
    {
        projectTile.second->SetRoom(nullptr);
    }
    _projectTiles.clear();

    _map.GetObjects().clear();

    RoomManager::GetInstance().Remove(_roomId);

    cout << "room is breaking shared_ptr_count" << shared_from_this().use_count() << endl;
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
    
    ServerProtocol::GameObjectType type = ObjectManager::GetObjectTypeById(gameObject->GetObjectId());
    
    switch (type)
    {
    case ServerProtocol::PLAYER: 
        {
            DoAsync(&Room::EnterGame_Player, dynamic_pointer_cast<Player>(gameObject));
        }
        break;
        case ServerProtocol::MONSTER: 
        {
             DoAsync(&Room::EnterGame_Monster, dynamic_pointer_cast<Monster>(gameObject));
        }
        break;
        case ServerProtocol::PROJECTTILE:
        {
            DoAsync(&Room::EnterGame_ProjectTile, dynamic_pointer_cast<ProjectTile>(gameObject));
        }
        break;
        case ServerProtocol::MAGIC :
        {
            DoAsync(&Room::EnterGame_ProjectTile, dynamic_pointer_cast<ProjectTile>(gameObject));
        }
        break;
    }
}

void Room::EnterGame_Player(PlayerRef player)
{
  
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
 

}

void Room::EnterGameEventSend_Monster(MonsterRef monster)
{
    if (monster == nullptr)
        return;

    ServerProtocol::S_SPAWN SpawnPacket;
    SpawnPacket.add_objects()->CopyFrom(monster->GetObjectInfo());
    auto SpawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(SpawnPacket);
    DoAsync(&Room::Broadcast,SpawnPacketBuffer);
}

void Room::EnterGameEventSend_ProjectTile(ProjectTileRef projecTtile)
{
    if (projecTtile == nullptr)
        return;

    ServerProtocol::S_SPAWN SpawnPacket;
    SpawnPacket.add_objects()->CopyFrom(projecTtile->GetObjectInfo());
    auto SpawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(SpawnPacket);
    DoAsync(&Room::Broadcast, SpawnPacketBuffer);
}

void Room::LeaveGame(int32 objectId)
{
    if (objectId < 0)
        return;

    ServerProtocol::GameObjectType type = ObjectManager::GetObjectTypeById(objectId);

    switch (type)
    {
        case ServerProtocol::PLAYER:
            DoAsync(&Room::LeaveGame_Player,objectId);
            break;
        case ServerProtocol::MONSTER:
            DoAsync(&Room::LeaveGame_Monster, objectId);
            break;
        case ServerProtocol::PROJECTTILE:
            DoAsync(&Room::LeaveGame_ProjectTile, objectId);
            break;
        case ServerProtocol::MAGIC:
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


    DoAsync([this, it]() {
        if (it->second == nullptr)
            return;

        _map.ApplyLeave(static_pointer_cast<GameObject>(it->second));

        LeaveGameEventSend_Player(it->second, it->second->GetObjectId());

        //플레이어 와 Room 의존 끊기
        it->second->SetRoom(nullptr);

        //플레이어 목록에서 삭제
        _players.erase(it);
    });
}

void Room::LeaveGame_Monster(int32 objectId)
{
    auto it = std::find_if(_monsters.begin(), _monsters.end(), [objectId](const std::pair<const int32, MonsterRef>& pair) {
        return pair.second->GetObjectInfo().objectid() == objectId;
    });

    if (it == _monsters.end())
        return;

    DoAsync([this, it]() {
        if (it->second == nullptr)
            return;

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

        if (it->second == nullptr)
            return;

        ProjectTileRef projecttile = static_pointer_cast<ProjectTile>(it->second);
        _map.ApplyLeave(projecttile);

        LeaveGameEventSend_ProjectTile(it->second, it->second->GetObjectId());

        it->second->SetRoom(nullptr);

        _projectTiles.erase(it);

        projecttile->SetOwner(nullptr);
    });
}

void Room::ExitGameEventSend(PlayerRef player)
{

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
    ServerProtocol::S_LEAVE_GAME leavePacket;
    leavePacket.set_exit(true);
    auto leavePacketBuffer = RoomPacketHandler::MakeSendBuffer(leavePacket);
    player->GetSession()->Send(leavePacketBuffer);

    //서버에 접속인원에게 퇴장 패킷 생성
    ServerProtocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    auto despawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
        if (objectid != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }
}

void Room::LeaveGameEventSend_Monster(MonsterRef monster, int32 objectid)
{
    //서버에 접속인원에게 퇴장 패킷 생성
    ServerProtocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    auto despawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
        if (objectid != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }
}

void Room::LeaveGameEventSend_ProjectTile(ProjectTileRef projectTile, int32 objectid)
{
    //서버에 접속인원에게 퇴장 패킷 생성
    ServerProtocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    auto despawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(despawnPacket);

    // 다른 플레이어에게만 패킷 전송 
    for (const auto& pair : _players) {
        if (objectid != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
    }
}

void Room::HandleMove(PlayerRef& player, ServerProtocol::C_MOVE& pkt)
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

void Room::HandleMoveEvent(PlayerRef player, ServerProtocol::C_MOVE pkt)
{
    //다른 플레이어에게도 이동 전달
    ServerProtocol::S_MOVE resMovePacket;
    resMovePacket.set_objectid(player->GetObjectInfo().objectid());
    resMovePacket.mutable_posinfo()->set_movedir(pkt.posinfo().movedir());
    resMovePacket.mutable_posinfo()->set_posx(pkt.posinfo().posx());
    resMovePacket.mutable_posinfo()->set_posy(pkt.posinfo().posy());
    resMovePacket.mutable_posinfo()->set_state(pkt.posinfo().state());

    auto resMovePacketBuffer = RoomPacketHandler::MakeSendBuffer(resMovePacket);
    for (const auto& pair : _players)
    {
        if (player != pair.second)
        {
            pair.second->GetSession()->Send(resMovePacketBuffer);
            cout << "이동 이벤트 전송 받은 플레이어 id" << pair.second->GetObjectInfo().objectid() << endl;
        }
    }
}

void Room::HandleSkill(PlayerRef& player, ServerProtocol::C_SKILL& pkt)
{
    if (player == nullptr)
        return;

    //외부에서 다른 곳에서  playerinfo가수정이될수도있기때문에 미리 정보를 받아둠
    ServerProtocol::OBJECT_INFO info = player->GetObjectInfo();
    if (info.posinfo().state() != ServerProtocol::CreatureState::IDLE)
        return;

    // TODO : 스킬 사용 가능 여부 체크

   //스킬 사용 애니메이션을 플레이어들에게 전송
    info.mutable_posinfo()->set_state(ServerProtocol::CreatureState::SKILL);

    ServerProtocol::S_SKILL skill;

    skill.set_objectid(info.objectid());
    skill.mutable_info()->set_skillid(pkt.info().skillid());
    auto resSkillPacketBuffer = RoomPacketHandler::MakeSendBuffer(skill);
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
    case ServerProtocol::SKILL_AUTO:
    {
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
        GameObjectRef target = _map.Find(skillPos);
        if (target != nullptr)
        {
            cout << "Hit GameObject !" << endl;
        }
    }
    break;
    case ServerProtocol::SKILL_PROJECTILE:
    {
        ArrowRef arrow = ObjectManager::GetInstance().Add<Arrow>();
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
        if (arrow == nullptr)
            return;

        //posinfo가아닌 object의 info의 posinfo에서 봐야함  
        arrow->SetOwner(player);
        arrow->SetSkillData(skillData);
        arrow->GetObjectInfo().mutable_posinfo()->set_state(ServerProtocol::MOVING);
        arrow->GetObjectInfo().mutable_posinfo()->set_movedir(player->GetMoveDir());
        arrow->GetObjectInfo().mutable_posinfo()->set_posx(skillPos.posx);
        arrow->GetObjectInfo().mutable_posinfo()->set_posy(skillPos.posy);
        arrow->SetSpeed(skillData.projectile.speed);

        GameObjectRef gameObject = static_pointer_cast<GameObject>(arrow);
        DoAsync(&Room::EnterGame, gameObject);
    }
    break;
    case ServerProtocol::SKILL_MAGIC:
    {
        MagicSkillRef magic = ObjectManager::GetInstance().Add<MagicSkill>();
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());

        if (magic == nullptr)
            return;

        magic->SetOwner(player);
        magic->SetSkillData(skillData);
        //magic->GetObjectInfo().mutable_posinfo()->set_state(ServerProtocol::MOVING);
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

