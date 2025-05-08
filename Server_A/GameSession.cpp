#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "ObjectManager.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "RedisManager.h"

GameSessionManager* GGameSessionManager;

void GameSession::InitPlayer()
{
    _myplayer = Make_Shared<Player>();
    _myplayer->SetSession(GetGameSessionRef());
}

void GameSession::OnConnected()
{
    GGameSessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
    cout << "클라이언트 소켓 연결 완료 " <<endl;

    //InitPlayer();
    //RoomRef room = RoomManager::GetInstance().Find(1);
    //room->EnterGame(_myplayer);

}

void GameSession::OnDisConnected()
{

    RoomRef room = RoomManager::GetInstance().Find(1);

    const char* query = "EXPIRE user_info:%s %d";
    RedisManager::GetInstance().RAsyncCommand(query, GetUserId().c_str(),60);

    GGameSessionManager->SessionClose(GetSessionId(),GetUserId());

    //_myplayer = nullptr;
    //GGameSessionManager->Remove(GetSessionId());
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    GameSessionRef session = GetGameSessionRef();
    RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);


    ClientPacketHandler::HandlePacket(session, buffer, len);
}