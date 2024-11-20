#include "pch.h"
#include "Room.h"
#include "Player.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"


Room::Room()
{
}

Room::~Room()
{
}

void Room::EnterGame(PlayerRef& newPlayer)
{
    if (newPlayer == NULL)
        return;

    WRITE_LOCK;
    {
        _players.push_back(newPlayer);
        newPlayer->SetRoom(shared_from_this());

        //본인 정보 전송 
        {
            Protocol::S_ENTER_GAME enterPacket;
            Protocol::PLAYER_INFO playerInfo = newPlayer->GetPlayerInfo();

            *enterPacket.mutable_player() = playerInfo;


            auto enterPacketBuffer = ClientPacketHandler::MakeSendBuffer(enterPacket);
            newPlayer->GetSession()->Send(enterPacketBuffer);

            Protocol::S_SPAWN SpawnPacket;
            for (PlayerRef p : _players)
            {
                if (p != newPlayer)
                {
                    Protocol::PLAYER_INFO* playersInfo = SpawnPacket.add_players();
                    *playersInfo = p->GetPlayerInfo();
                }
            }
            auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
            newPlayer->GetSession()->Send(SpawnPacketBuffer);
        }


        //타인 정보 전송
        {
            Protocol::S_SPAWN SpawnPacket;

            for (PlayerRef p : _players) {
                Protocol::PLAYER_INFO* playersInfo = SpawnPacket.add_players();
                *playersInfo = p->GetPlayerInfo();
            }

            auto SpawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(SpawnPacket);
            for (PlayerRef p : _players)
            {
                if (newPlayer != p)
                    p->GetSession()->Send(SpawnPacketBuffer);
            }
        }
    }
}

void Room::LeaveGame(int PlayerId)
{
    WRITE_LOCK;
    {
        auto it = std::find_if(_players.begin(), _players.end(), [PlayerId](const PlayerRef& player) {
            return player->GetPlayerInfo().playerid() == PlayerId;
        });

        if (it == _players.end())
            return;
    
        (*it)->SetRoom(nullptr);

        //본인에게 정보 전송 
        {
            Protocol::S_LEAVE_GAME leavePacket;
            leavePacket.set_exit(true);
            auto leavePacketBuffer = ClientPacketHandler::MakeSendBuffer(leavePacket);
            (*it)->GetSession()->Send(leavePacketBuffer);
        }

        //타인에게 정보 전송
        {
            Protocol::S_DESPAWN despawnPacket;
            despawnPacket.add_playerids((*it)->GetPlayerInfo().playerid());
            auto despawnPacketBuffer = ClientPacketHandler::MakeSendBuffer(despawnPacket);
            for (PlayerRef p : _players)
            {
                if((*it) != p)
                    p->GetSession()->Send(despawnPacketBuffer);
            }
        }
    }
}

void Room::Broadcast(SendBufferRef sendBuffer)
{
    WRITE_LOCK
    {
        for (PlayerRef p : _players)
        {
            p->GetSession()->Send(sendBuffer);
        }
    }
}

void Room::HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt)
{
    if (player == nullptr)
        return;

    WRITE_LOCK
    {
        //서버에서 좌표이동 처리 c++특성상 프로토버퍼에 playerInfo 메세지 패킷을 한번에 담는 방법이없다고한다.. 포인터 안전성 고려 떄문인 듯
        Protocol::PLAYER_INFO& playerInfo = player->GetPlayerInfo();
        playerInfo.mutable_posinfo()->set_movedir(pkt.posinfo().movedir());
        playerInfo.mutable_posinfo()->set_posx(pkt.posinfo().posx());
        playerInfo.mutable_posinfo()->set_posy(pkt.posinfo().posy());
        playerInfo.mutable_posinfo()->set_state(pkt.posinfo().state());

        //다른 플레이어에게도 이동 전달
        Protocol::S_MOVE resMovePacket;
        resMovePacket.set_playerid(player->GetPlayerInfo().playerid());
        resMovePacket.mutable_posinfo()->set_movedir(pkt.posinfo().movedir());
        resMovePacket.mutable_posinfo()->set_posx(pkt.posinfo().posx());
        resMovePacket.mutable_posinfo()->set_posy(pkt.posinfo().posy());
        resMovePacket.mutable_posinfo()->set_state(pkt.posinfo().state());

        auto resMovePacketBuffer = ClientPacketHandler::MakeSendBuffer(resMovePacket);
        Broadcast(resMovePacketBuffer);
    }
}

void Room::HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt)
{
    if (player == nullptr)
        return;

    WRITE_LOCK
    {
        //외부에서 다른 곳에서  playerinfo가수정이될수도있기때문에 미리 정보를 받아둠
        Protocol::PLAYER_INFO info = player->GetPlayerInfo();
        if (info.posinfo().state() != Protocol::CreatureState::IDLE)
        return;

        // TODO : 스킬 사용 가능 여부 체크

        //통과
        info.mutable_posinfo()->set_state(Protocol::CreatureState::SKILL);

        Protocol::S_SKILL skill;

        skill.set_playerid(info.playerid());
        skill.mutable_info()->set_skillid(1);
        auto resSkillPacketBuffer = ClientPacketHandler::MakeSendBuffer(skill);
        Broadcast(resSkillPacketBuffer);

        // TODO : 데미지 판정
    }
}
