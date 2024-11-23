#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "MapManager.h"

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


void Room::EnterGame(PlayerRef& newPlayer)
{
    if (newPlayer == NULL)
        return;

    WRITE_LOCK;
    {
        _players.insert(make_pair(newPlayer->GetObjectInfo().objectid(), newPlayer));
        newPlayer->SetRoom(shared_from_this());

        //본인에게 본인 정보 및 현재 룸에 있는 인원 전송 
        {
            //본인정보 전송
            Protocol::S_ENTER_GAME enterPacket;
            Protocol::OBJECT_INFO playerInfo = newPlayer->GetObjectInfo();

            *enterPacket.mutable_player() = playerInfo;


            auto enterPacketBuffer = ClientPacketHandler::MakeSendBuffer(enterPacket);
            newPlayer->GetSession()->Send(enterPacketBuffer);

            //룸에 존재하는 인원들 정보 새로 접속한 인원에게 전송
            Protocol::S_SPAWN SpawnPacket;
            for (const auto& playerPair : _players)
            {
               const PlayerRef& p = playerPair.second;
                 if (p != newPlayer)
                 {
                     Protocol::OBJECT_INFO* playersInfo = SpawnPacket.add_objects();
                     *playersInfo = p->GetObjectInfo();
                 }
                
            }
            auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
            newPlayer->GetSession()->Send(SpawnPacketBuffer);
        }


        //이미 룸에 존재하던 인원에게 새로운 인원 정보 전송
        {
            Protocol::S_SPAWN SpawnPacket;
            SpawnPacket.add_objects()->CopyFrom(newPlayer->GetObjectInfo());
         
            auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
            for (const auto& playerPair : _players)
            {
                if (newPlayer != playerPair.second)
                    playerPair.second->GetSession()->Send(SpawnPacketBuffer);
            }
        }
    }
}

void Room::LeaveGame(int PlayerId)
{
    WRITE_LOCK;
    {
        auto it = std::find_if(_players.begin(), _players.end(), [PlayerId](const std::pair<const int32, PlayerRef>& pair) {
            return pair.second->GetObjectInfo().objectid() == PlayerId;
        });

        if (it == _players.end())
            return;
    

        // 삭제할 플레이어를 저장
        PlayerRef playerToRemove = it->second;

        // 플레이어 목록에서 삭제
        _players.erase(it);

        // 자신에게 퇴장 패킷 전송
        Protocol::S_LEAVE_GAME leavePacket;
        leavePacket.set_exit(true);
        auto leavePacketBuffer = ClientPacketHandler::MakeSendBuffer(leavePacket);
        playerToRemove->GetSession()->Send(leavePacketBuffer);

        // 다른 플레이어에게 소멸 패킷 전송
        Protocol::S_DESPAWN despawnPacket;
        despawnPacket.add_playerids(playerToRemove->GetObjectInfo().objectid());
        auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);

        // 다른 플레이어에게만 패킷 전송 
        for (const auto& pair : _players) {
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
        _map.ApplyMove(player,Vector2Int(movePosInfo.posx(), movePosInfo.posy()));
      

        //다른 플레이어에게도 이동 전달
        Protocol::S_MOVE resMovePacket;
        resMovePacket.set_playerid(player->GetObjectInfo().objectid());
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

        skill.set_playerid(info.objectid());
        skill.mutable_info()->set_skillid(pkt.info().skillid());
        auto resSkillPacketBuffer = ClientPacketHandler::MakeSendBuffer(skill);
        Broadcast(resSkillPacketBuffer);

        // 현재는 스킬이 2가지 바께 없어서if로 분기하지만 스킬이많아진다면 새로 클래스를 만들어서 분기해야할듯하다.
        if (pkt.info().skillid() == 1)
        {
            //평타 데미지 판정
            Vector2Int skillPos = player->GetFrontCellPos(info.posinfo().movedir());
            PlayerRef target = _map.Find(skillPos);
            if (target != nullptr)
            {
                cout << "Hit player !" << endl;
            }
        }
        else if (pkt.info().skillid() == 2)
        {
            //TODO : Arrow

        }
    }
}

