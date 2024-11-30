#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "ObjectManager.h"
#include "Player.h"
#include "RoomManager.h"
#include "DataManager.h"

GameSessionManager* SessionManager;

void GameSession::OnConnected()
{
    SessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
    cout << "접속 인원 추가" << endl;
    
    //세션에 누군가 할당이 되었는데 원래는 DB에서 플레이어의 정보를 긁어와야하는데..
    
    //세션도 해당 플레이어의 정보를 가지고 있는다.
    ObjectManager& ObjectManager = ObjectManager::GetInstance();
    _myplayer = ObjectManager.Add<Player>();
    {
        _myplayer->GetObjectInfo().set_name("Player_"+ to_string(_myplayer->GetObjectInfo().objectid()));
        _myplayer->SetState(Protocol::CreatureState::IDLE);
        _myplayer->SetMoveDir(Protocol::MoveDir::DOWN);
        _myplayer->SetPosx(0);
        _myplayer->SetPosy(0);
        _myplayer->SetSession(static_pointer_cast<GameSession>(shared_from_this()));

        auto it = std::find_if(DataManager::GetInstance().GetStatDict().begin(), DataManager::GetInstance().GetStatDict().end(), 
            [](const std::pair<const int32, Protocol::STATINFO>& pair) {
            return pair.second.level() == 1;
        });

        if (it == DataManager::GetInstance().GetStatDict().end())
            return;
        _myplayer->SetObjectStat(it->second);
    }

    RoomManager& RoomManager = RoomManager::GetInstance();
    RoomManager.Find(1)->EnterGame(static_pointer_cast<GameObject>(_myplayer));
   
}

void GameSession::OnDisConnected()
{
    RoomManager& RoomManager = RoomManager::GetInstance();
    RoomManager.Find(1)->LeaveGame(_myplayer->GetObjectInfo().objectid());

    SessionManager->Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    //TODO : 클라이언트 패킷 핸들러 작성
    ClientPacketHandler::HandlePacket(session, buffer, len);
}