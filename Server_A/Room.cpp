#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "MapManager.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "Arrow.h"
#include "Monster.h"
#include "DataManager.h"
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

void Room::TileUpdate()
{
    WRITE_LOCK
    {
        for (const auto& pair : _projectTiles)
        {
            pair.second->Update();
        }
    }
}


void Room::EnterGame(GameObjectRef gameObject)
{
    if (gameObject == nullptr)
        return;

    Protocol::GameObjectType type = ObjectManager::GetObjectTypeById(gameObject->GetObjectId());

    WRITE_LOCK;
    {
        switch (type)
        {
            case Protocol::PLAYER :
            {
                PlayerRef player = dynamic_pointer_cast<Player>(gameObject);

                _players.insert(make_pair(player->GetObjectId(), player));
                player->SetRoom(shared_from_this());

                //본인에게 본인 정보 및 현재 룸에 있는 인원 전송 
                {
                    //본인정보 전송
                    Protocol::S_ENTER_GAME enterPacket;
                    Protocol::OBJECT_INFO playerInfo = player->GetObjectInfo();

                    *enterPacket.mutable_player() = playerInfo;


                    auto enterPacketBuffer = ClientPacketHandler::MakeSendBuffer(enterPacket);
                    player->GetSession()->Send(enterPacketBuffer);

                    //룸에 존재하는 인원들 정보 새로 접속한 인원에게 전송
                    Protocol::S_SPAWN SpawnPacket;
                    for (const auto& playerPair : _players)
                    {
                        const PlayerRef& p = playerPair.second;
                        if (p != player)
                        {
                            Protocol::OBJECT_INFO* playersInfo = SpawnPacket.add_objects();
                            *playersInfo = p->GetObjectInfo();
                        }

                    }
                    auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
                    player->GetSession()->Send(SpawnPacketBuffer);
                }
                break;
            }
            case Protocol::MONSTER :
            {
                MonsterRef monster = dynamic_pointer_cast<Monster>(gameObject);
                _monsters.insert(make_pair(monster->GetObjectId(), monster));
                monster->SetRoom(shared_from_this());
            }
            break;
            case Protocol::PROJECTTILE :
            {
                ProjectTileRef projectTile = dynamic_pointer_cast<ProjectTile>(gameObject);
                _projectTiles.insert(make_pair(projectTile->GetObjectId(), projectTile));
                projectTile->SetRoom(shared_from_this());
            }
            break;
        }

        //이미 룸에 존재하던 인원에게 새로운 인원 정보 전송
        {
            Protocol::S_SPAWN SpawnPacket;
            SpawnPacket.add_objects()->CopyFrom(gameObject->GetObjectInfo());
         
            auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
            for (const auto& playerPair : _players) //오브젝트의 경우에는 모든 플레이어에게 다보내게 됨
            {
                if (gameObject->GetObjectId() != playerPair.second->GetObjectId())
                    playerPair.second->GetSession()->Send(SpawnPacketBuffer);
            }
        }
    }
}

void Room::LeaveGame(int objectId)
{

    Protocol::GameObjectType type = ObjectManager::GetObjectTypeById(objectId);

    WRITE_LOCK;
    {
        switch (type)
        {
            case Protocol::PLAYER:
            {
                auto it = std::find_if(_players.begin(), _players.end(), [objectId](const std::pair<const int32, PlayerRef>& pair) {
                    return pair.second->GetObjectInfo().objectid() == objectId;
                });

                if (it == _players.end())
                    return;


                // 삭제할 플레이어를 저장
                PlayerRef playerToRemove = it->second;
                it->second->SetRoom(nullptr);
                //삭제 전 맵관리에서도 해당 객체는 사라젔다고 알려야한다.
                _map.ApplyLeave(static_pointer_cast<GameObject>(playerToRemove));
                // 플레이어 목록에서 삭제
                _players.erase(it);

                // 자신에게 퇴장 패킷 전송
                Protocol::S_LEAVE_GAME leavePacket;
                leavePacket.set_exit(true);
                auto leavePacketBuffer = ClientPacketHandler::MakeSendBuffer(leavePacket);
                playerToRemove->GetSession()->Send(leavePacketBuffer);
            }
                break;
            case Protocol::MONSTER :
            {
                auto it = std::find_if(_monsters.begin(), _monsters.end(), [objectId](const std::pair<const int32, MonsterRef>& pair) {
                    return pair.second->GetObjectInfo().objectid() == objectId;
                });

                if (it == _monsters.end())
                    return;

                MonsterRef  monsterToRemove = it->second;
                it->second->SetRoom(nullptr);
                _map.ApplyLeave(static_pointer_cast<GameObject>(monsterToRemove));
                _monsters.erase(it);
            }
                break;
            case Protocol::PROJECTTILE:
            {
                auto it = std::find_if(_projectTiles.begin(), _projectTiles.end(), [objectId](const std::pair<const int32, ProjectTileRef>& pair) {
                    return pair.second->GetObjectInfo().objectid() == objectId;
                });

                if (it == _projectTiles.end())
                    return;

                ProjectTileRef  tileToRemove = it->second;
                it->second->SetRoom(nullptr);
                _projectTiles.erase(it);
            }
                break;

        }
 
        // 다른 플레이어에게 소멸 패킷 전송
        Protocol::S_DESPAWN despawnPacket;
        despawnPacket.add_objectids(objectId);
        auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);

        // 다른 플레이어에게만 패킷 전송 
        for (const auto& pair : _players) {
            if (objectId != pair.second->GetObjectId())
            pair.second->GetSession()->Send(despawnPacketBuffer);
        }
    }
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK
    {
        for (const auto& playerPair : _players)
        {
            playerPair.second->GetSession()->Send(sendBuffer);
            cout <<"Broadcast 전송된 플레이어 id" << playerPair.second->GetObjectInfo().objectid() << endl;
        }
    }
}

void Room::HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt)
{
    if (player == nullptr)
        return;

    WRITE_LOCK
    {
        // TODO : 검증

        //서버에서 좌표이동 처리 c++특성상 프로토버퍼에 playerInfo 메세지 패킷을 한번에 담는 방법이없다고한다.. 포인터 안전성 고려 떄문인 듯
        Protocol::POSITIONINFO movePosInfo = pkt.posinfo();
        Protocol::OBJECT_INFO& info = player->GetObjectInfo();

        if (movePosInfo.posx() != info.mutable_posinfo()->posx() || movePosInfo.posy() != info.mutable_posinfo()->posy())
        {
            if(_map.CanGo(Vector2Int(movePosInfo.posx(),movePosInfo.posy())) == false)
                return;
        }

        info.mutable_posinfo()->set_state(movePosInfo.state());
        info.mutable_posinfo()->set_movedir(movePosInfo.movedir());
        _map.ApplyMove(static_pointer_cast<GameObject>(player),Vector2Int(movePosInfo.posx(), movePosInfo.posy()));
      

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
}

void Room::HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt)
{
    if (player == nullptr)
        return;

    WRITE_LOCK
    {
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

        // TODO : c++는 unordered map에 반복자를 기본적으로 지원하지않는다.. 나중에 함수로 뺴서 기능 구현 미리 해놓는게 좋을듯.
        auto it = std::find_if(DataManager::GetInstance().GetSkillDict().begin(), DataManager::GetInstance().GetSkillDict().end(),
            [pkt](const std::pair<const int32, Skill>& pair) {
            return pair.first == pkt.info().skillid();
        });

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
                    EnterGame(gameObject);
                }
            break;
        }
    }
}

