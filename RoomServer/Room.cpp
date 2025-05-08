#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "ServerProtocol.pb.h"
#include "RoomPacketHandler.h"
#include "UserServerSession.h"
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
    RedisManager::GetInstance().RAsyncCommand(query, _roomId);

    query = "DEL room_user:%d";
    RedisManager::GetInstance().RAsyncCommand(query, _roomId);

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
    DoAsync(&Room::BroadcastExcept, std::move(exitPacketBuffer), std::move(player));

    HashMap<string, ServerConfigData> dict = DataManager::GetInstance().GetServerConfigDict();

    std::string host = "http://" + dict["database"].nodeData.host + ":" + dict["database"].nodeData.port;

    httplib::Client cli(host);
    cli.set_connection_timeout(5); // 5초
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

    GRoomManager->Remove(_roomId);

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

PlayerRef Room::GetPlayer(string userid)
{
    auto it = _useridToPlayers.find(userid);

    if (it == _useridToPlayers.end())
        return nullptr;

    return it->second;
}

PlayerRef Room::GetPlayer(int32 objectid)
{
    auto it = _players.find(objectid);

    if (it == _players.end())
        return nullptr;

    return it->second;
}


void Room::EnterGame(GameObjectRef gameObject)
{
    if (gameObject == nullptr)
        return;
    
    Common::GameObjectType type = ObjectManager::GetObjectTypeById(gameObject->GetObjectId());
    
    switch (type)
    {
    case Common::PLAYER:
        {
            DoAsync(&Room::EnterGame_Player, dynamic_pointer_cast<Player>(gameObject));
        }
        break;
        case Common::MONSTER:
        {
             DoAsync(&Room::EnterGame_Monster, dynamic_pointer_cast<Monster>(gameObject));
        }
        break;
        case Common::PROJECTTILE:
        {
            DoAsync(&Room::EnterGame_ProjectTile, dynamic_pointer_cast<ProjectTile>(gameObject));
        }
        break;
        case Common::MAGIC :
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
        SpawnMonster(5, 5);
        SpawnMonster(8, 3);
        SpawnMonster(10, 7);

        tempSpawnHandle = true;
    }


    DoAsync([this, player]() {
        _players.insert(std::make_pair(player->GetObjectId(), player));
        player->SetRoom(GetSharedRoomPtr());
        _map.ApplyMove(static_pointer_cast<GameObject>(player), Vector2Int(player->GetPosx(), player->GetPosy()));
        EnterGameEventSend_Player(player);
    });

    const char* query = "HSET room_score:%d:%s nickname %s";
    RedisManager::GetInstance().RAsyncCommand(query, GetRoomId(), player->GetUserId().c_str(), player->GetUserNickName().c_str());
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
    ServerProtocol::S_ENTER_GAME enterPacket;
    enterPacket.set_roomid(_roomId);
    enterPacket.set_roomname(_roomName);
    enterPacket.set_userid(player->GetUserId());
    enterPacket.mutable_player()->CopyFrom(player->GetObjectInfo());
    auto enterPacketBuffer = RoomPacketHandler::MakeSendBuffer(enterPacket);
    DoAsync(std::bind(&Room::Send, this, player->GetSession(), enterPacketBuffer));

    //룸에 존재하는 인원들 정보를 새로 접속한 인원에게 전송
    {
        ServerProtocol::S_SPAWN SpawnPacket;
        SpawnPacket.set_userid(player->GetUserId());
        SpawnPacket.set_roomid(GetRoomId());
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

        auto SpawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(SpawnPacket);
        DoAsync(std::bind(&Room::Send, this, player->GetSession(), SpawnPacketBuffer));
    }

    //이미 룸에 존재하던 인원에게 새로운 인원 정보 전송
    {
        ServerProtocol::S_SPAWN SpawnPacket;
        SpawnPacket.add_objects()->CopyFrom(player->GetObjectInfo());
        SpawnPacket.set_roomid(GetRoomId());

        auto SpawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(SpawnPacket);

        BroadcastExcept(SpawnPacketBuffer,player);
    }
}

void Room::EnterGameEventSend_Monster(MonsterRef monster)
{
    if (monster == nullptr)
        return;
    ServerProtocol::S_SPAWN SpawnPacket;
    SpawnPacket.add_objects()->CopyFrom(monster->GetObjectInfo());
    SpawnPacket.set_roomid(GetRoomId());
    auto SpawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(SpawnPacket);
    DoAsync(&Room::Broadcast,std::move(SpawnPacketBuffer));
}

void Room::EnterGameEventSend_ProjectTile(ProjectTileRef projecTtile)
{
    if (projecTtile == nullptr)
        return;

    ServerProtocol::S_SPAWN SpawnPacket;
    SpawnPacket.add_objects()->CopyFrom(projecTtile->GetObjectInfo());
    SpawnPacket.set_roomid(GetRoomId());
    auto SpawnPacketBuffer = RoomPacketHandler::MakeSendBuffer(SpawnPacket);
    DoAsync(&Room::Broadcast, std::move(SpawnPacketBuffer));
}

void Room::LeaveGame(int32 objectId)
{
    if (objectId < 0)
        return;

    Common::GameObjectType type = ObjectManager::GetObjectTypeById(objectId);

    switch (type)
    {
        case Common::PLAYER:
            DoAsync(&Room::LeaveGame_Player,std::move(objectId));
            break;
        case Common::MONSTER:
            DoAsync(&Room::LeaveGame_Monster, std::move(objectId));
            break;
        case Common::PROJECTTILE:
            DoAsync(&Room::LeaveGame_ProjectTile, std::move(objectId));
            break;
        case Common::MAGIC:
            DoAsync(&Room::LeaveGame_ProjectTile, std::move(objectId));
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

    //it는 해당 객체가있는 위치를 가르키는 포인터기 떄문에 비동기로 나중에 사용될 객체는 따로 받아서 람다에 복사해서 사용하자.
    PlayerRef player = it->second;

    DoAsync([this, player]() {
        if (player == nullptr)
            return;

        _map.ApplyLeave(static_pointer_cast<GameObject>(player));

        LeaveGameEventSend_Player(player, player->GetObjectId());

        //플레이어 와 Room 의존 끊기
        player->SetRoom(nullptr);

        //플레이어 목록에서 삭제
        _players.erase(player->GetObjectId());
    });
}

void Room::LeaveGame_Monster(int32 objectId)
{
    auto it = std::find_if(_monsters.begin(), _monsters.end(), [objectId](const std::pair<const int32, MonsterRef>& pair) {
        return pair.second->GetObjectInfo().objectid() == objectId;
    });

    if (it == _monsters.end())
        return;

    MonsterRef monster = it->second;

    DoAsync([this, monster]() {
        if (monster == nullptr)
            return;

        _map.ApplyLeave(static_pointer_cast<Monster>(monster));

        LeaveGameEventSend_Monster(monster, monster->GetObjectId());

        monster->SetRoom(nullptr);

        _monsters.erase(monster->GetObjectId());
    });
}

void Room::LeaveGame_ProjectTile(int32 objectId)
{
    auto it = std::find_if(_projectTiles.begin(), _projectTiles.end(), [objectId](const std::pair<const int32, ProjectTileRef>& pair) {
        return pair.second->GetObjectInfo().objectid() == objectId;
    });

    if (it == _projectTiles.end())
        return;

    ProjectTileRef tile = it->second;

    DoAsync([this, tile]() {

        if (tile == nullptr)
            return;

        _map.ApplyLeave(tile);

        LeaveGameEventSend_ProjectTile(tile, tile->GetObjectId());

        tile->SetRoom(nullptr);

        _projectTiles.erase(tile->GetObjectId());

        tile->SetOwner(nullptr);
    });
}

void Room::ExitGameEventSend(PlayerRef player)
{
    //게임 방에서 완전히 나가기가 완료되었다고 User서버에게 패킷을 전송해주자
    ServerProtocol::S_EXIT_GAME packet;
    packet.set_exitflag(true);
    packet.set_objectid(player->GetObjectId());
    packet.set_roomid(GetRoomId());
    auto exitPacketBuffer = RoomPacketHandler::MakeSendBuffer(packet);
    player->GetSession()->Send(exitPacketBuffer);

    //해당방의 score 삭제 처리 1.nickname 2.kill 3.death 4.nickname Del로 그냥 모든 필드 밀어줌 필요시 HDell로 필요한 필드만 삭제

    const char* query = "DEL room_score:%d:%s";
    RedisManager::GetInstance().RAsyncCommand(query, GetRoomId(), player->GetUserId().c_str());

    query = "SREM room_user:%d %s";
    RedisManager::GetInstance().RAsyncCommand(query, _roomId, player->GetUserId().c_str());

    //room을 만든 user가 방에서 나감
    if (player->GetUserNickName() == GetRootUser())
        DoAsync(&Room::RoomBreak, std::move(player));
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
    if (_players.size() == 0)
        return;

    Set<PacketSessionRef> sessions;

    for (const auto& pair : _players)
    {
        sessions.insert(pair.second->GetSession());
    }

    for (const auto& session : sessions)
    {
        session->Send(sendBuffer);
    }
}

void Room::BroadcastExcept(SendBufferRef sendBuffer, PlayerRef player)
{
    if (_players.size() == 0)
        return;

    Set<PacketSessionRef> sessions;

    for (const auto& pair : _players)
    {
        if (pair.second != player)
        {
            sessions.insert(pair.second->GetSession());
        }
    }
    for (const auto& session : sessions)
    {
        session->Send(sendBuffer);
    }
}

void Room::Send(PacketSessionRef session, SendBufferRef sendBuffer)
{
    session->Send(sendBuffer);
}

void Room::SpawnMonster(int32 y, int32 x)
{
    // TEMP 몬스터 소환
    MonsterRef monster = _objmanagers.Add<Monster>();
    monster->SetCellPos(x, y);

    GameObjectRef gameObject = static_pointer_cast<GameObject>(monster);
    EnterGame(gameObject);
}

void Room::LeaveGameEventSend_Player(PlayerRef player,int32 objectid)
{
    //본인에게 퇴장 패킷 전송
    ServerProtocol::S_LEAVE_GAME leavePacket;
    leavePacket.set_exit(true);
    leavePacket.set_objectid(objectid);
    leavePacket.set_objectid(GetRoomId());
    auto leavePacketBuffer = RoomPacketHandler::MakeSendBuffer(leavePacket);
    player->GetSession()->Send(leavePacketBuffer);

    //서버에 접속인원에게 퇴장 패킷 생성
    ServerProtocol::S_DESPAWN despawnPacket;
    despawnPacket.add_objectids(objectid);
    despawnPacket.set_roomid(GetRoomId());
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
    despawnPacket.set_roomid(GetRoomId());
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
    despawnPacket.set_roomid(GetRoomId());
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
    resMovePacket.mutable_posinfo()->CopyFrom(pkt.posinfo());
    resMovePacket.set_roomid(GetRoomId());
    auto resMovePacketBuffer = RoomPacketHandler::MakeSendBuffer(resMovePacket);
    
    Set<PacketSessionRef> sessions;
  
    BroadcastExcept(resMovePacketBuffer,player);
}

void Room::HandleSkill(PlayerRef& player, ServerProtocol::C_SKILL& pkt)
{
    if (player == nullptr)
        return;

    //외부에서 다른 곳에서  playerinfo가수정이될수도있기때문에 미리 정보를 받아둠
    Common::OBJECT_INFO info = player->GetObjectInfo();
    if (info.posinfo().state() != Common::CreatureState::IDLE)
        return;

    // TODO : 스킬 사용 가능 여부 체크

   //스킬 사용 애니메이션을 플레이어들에게 전송
    info.mutable_posinfo()->set_state(Common::CreatureState::SKILL);

    ServerProtocol::S_SKILL skill;

    skill.set_objectid(info.objectid());
    skill.mutable_info()->set_skillid(pkt.info().skillid());
    skill.set_roomid(GetRoomId());
    auto resSkillPacketBuffer = RoomPacketHandler::MakeSendBuffer(skill);
    DoAsync(&Room::Broadcast, std::move(resSkillPacketBuffer));

    auto it = DataManager::GetInstance().GetSkillDict().find(pkt.info().skillid());

    if (it == DataManager::GetInstance().GetSkillDict().end())
        return;

    Skill skillData = it->second;

    switch (skillData.skillType)
    {
    case Common::SKILL_AUTO:
    {
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
        GameObjectRef target = _map.Find(skillPos);
        if (target != nullptr)
        {
            cout << "Hit GameObject !" << endl;
        }
    }
    break;
    case Common::SKILL_PROJECTILE:
    {
        ArrowRef arrow = _objmanagers.Add<Arrow>();
        Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
        if (arrow == nullptr)
            return;

        //posinfo가아닌 object의 info의 posinfo에서 봐야함  
        arrow->SetOwner(player);
        arrow->SetSkillData(skillData);
        arrow->GetObjectInfo().mutable_posinfo()->set_state(Common::MOVING);
        arrow->GetObjectInfo().mutable_posinfo()->set_movedir(player->GetMoveDir());
        arrow->GetObjectInfo().mutable_posinfo()->set_posx(skillPos.posx);
        arrow->GetObjectInfo().mutable_posinfo()->set_posy(skillPos.posy);
        arrow->SetSpeed(skillData.projectile.speed);

        GameObjectRef gameObject = static_pointer_cast<GameObject>(arrow);
        DoAsync(&Room::EnterGame, std::move(gameObject));
    }
    break;
    case Common::SKILL_MAGIC:
    {
        MagicSkillRef magic = _objmanagers.Add<MagicSkill>();
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
        DoAsync(&Room::EnterGame, std::move(gameObject));
    }
    break;
    }

}

