#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "PlayerManager.h"
#include "Player.h"
#include "RoomManager.h"

GameSessionManager* SessionManager;

void GameSession::OnConnected()
{
    SessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
    cout << "접속 인원 추가" << endl;
    
    //세션에 누군가 할당이 되었는데 원래는 DB에서 플레이어의 정보를 긁어와야하는데..
    
    //세션도 해당 플레이어의 정보를 가지고 있는다.
    PlayerManager& PlayerManager = PlayerManager::GetInstance();
    _myplayer = PlayerManager.Add();
    {
        _myplayer->GetPlayerInfo().set_name("Player_"+ to_string(_myplayer->GetPlayerInfo().playerid()));
        _myplayer->GetPlayerInfo().mutable_posinfo()->set_state(Protocol::CreatureState::IDLE);
        _myplayer->GetPlayerInfo().mutable_posinfo()->set_movedir(Protocol::MoveDir::None);
        _myplayer->GetPlayerInfo().mutable_posinfo()->set_posx(0);
        _myplayer->GetPlayerInfo().mutable_posinfo()->set_posy(0);
        _myplayer->SetSession(static_pointer_cast<GameSession>(shared_from_this()));
    }

    RoomManager& RoomManager = RoomManager::GetInstance();
    RoomManager.Find(1)->EnterGame(_myplayer);
   
}

void GameSession::OnDisConnected()
{
    RoomManager& RoomManager = RoomManager::GetInstance();
    RoomManager.Find(1)->LeaveGame(_myplayer->GetPlayerInfo().playerid());

    SessionManager->Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    //TODO : 클라이언트 패킷 핸들러 작성
    ClientPacketHandler::HandlePacket(session, buffer, len);
}