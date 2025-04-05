#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ClientPacketHandler.h"
#include "Player.h"
#include "ObjectManager.h"
#include "RoomManager.h"
#include "DataManager.h"
#include "RedisConnection.h"

GameSessionManager* SessionManager;

void GameSession::InitPlayer()
{
    _myplayer = ObjectManager::GetInstance().Add<Player>();
    {
        _myplayer->GetObjectInfo().set_name("Player_"+to_string(_myplayer->GetObjectInfo().objectid()));
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
}

void GameSession::OnConnected()
{
    SessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
    cout << "클라이언트 소켓 연결 완료 " <<endl;

    InitPlayer();
    //RoomRef room = RoomManager::GetInstance().Find(1);
    //room->EnterGame(_myplayer);

}

void GameSession::OnDisConnected()
{

    RoomRef room = RoomManager::GetInstance().Find(1);
    int32 objectid = _myplayer->GetObjectId();

    const char* query = "HDEL active_user %s";
    RedisUtils::RAsyncCommand(GRedisConnection->GetContext(), query, _userid.c_str());

    _myplayer = nullptr;

    if (room != nullptr)
        room->DoAsync(&Room::LeaveGame, objectid);
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);


    ClientPacketHandler::HandlePacket(session, buffer, len);
}