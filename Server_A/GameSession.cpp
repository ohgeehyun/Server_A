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
    RoomRef room = RoomManager::GetInstance().Find(1);
    room->EnterGame(_myplayer);

}

void GameSession::OnDisConnected()
{

    RoomRef room = RoomManager::GetInstance().Find(1);
    room->DoAsync(&Room::LeaveGame,_myplayer->GetObjectId());

    redisAsyncCommand(GRedisConnection->GetContext(), [](redisAsyncContext* context, void* reply, void* privdata)
    {
        RedisUtils::replyResponseHandler(reply,"Redis delete active user : ");

    }, nullptr, "HDEL active_user %s", _userid.c_str());

    _myplayer = nullptr;
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);

    //TODO : 만약 jwt토큰 검증이 패킷을조립전에 해야한다면 여기서 할 것 
    ClientPacketHandler::HandlePacket(session, buffer, len);
}