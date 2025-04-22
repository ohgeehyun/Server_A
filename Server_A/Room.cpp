#include "pch.h"
#include "Room.h"
#include "GameObject.h"
#include "Player.h"
#include "Monster.h"
#include "Arrow.h"
#include "Protocol.pb.h"
#include "ServerProtocol.pb.h"
#include "ClientPacketHandler.h"
#include "ServerPacketHandler.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "DataManager.h"
#include "MagicSkill.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include  "ServerSession.h"
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

    cout << _roomId << " 번 방 소멸자 호출 완료." << endl;
}

void Room::EnterGame(int32 roomId, GameObjectRef object, GameSessionRef ClientSession)
{
    if (object == nullptr)
        return;
    if (ClientSession == nullptr)
        return;
    
    //현재 유저서버에서 만들어진 적이 없는 방이거나 저장된 룸 아이디에 맞는 세션이없다면 서버로직중 예외 발생했거나 말도안되는 룸id를 패킷으로 받은 것 
    ServerSessionRef RoomServerSession = RoomManager::GetInstance().FindToSession(roomId);
    if (RoomServerSession)
        return;

    ServerProtocol::C_ENTER_GAME pkt;
    pkt.set_rommid(roomId);
    auto Packet = ServerPacketHandler::MakeSendBuffer(pkt);
    RoomServerSession->Send(Packet);
}


