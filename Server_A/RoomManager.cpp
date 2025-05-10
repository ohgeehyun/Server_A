#include "pch.h"
#include "Room.h"
#include "RoomManager.h"
#include "RedisConnection.h"
#include "ServerProtocol.pb.h"
#include "RoomSessionManager.h"
#include "ServerPacketHandler.h"
#include "RoomSession.h"
#include "JsonUtils.h"
#include "RedisUtils.h"


RoomRef RoomManager::Add(const ServerProtocol::S_CREATE_ROOM& pkt)
{
    RoomRef gameRoom = Make_Shared<Room>();
    
    WRITE_LOCK
    {
        gameRoom->SetRoomId(pkt.roomid());
        gameRoom->SetRoomName(pkt.roomname());
        gameRoom->SetRootUser(pkt.rootuser());
        gameRoom->SetPwdYn(pkt.pwdyn());

        if (pkt.pwdyn())
            gameRoom->SetRoomPwd(pkt.roompwd());
        else
            gameRoom->SetRoomPwd("");

        _rooms.insert(std::pair(pkt.roomid(),gameRoom));

    }
    return gameRoom;
}

bool RoomManager::Remove(int32 roomId)
{
  
    return _rooms.erase(roomId);
    
}

RoomRef RoomManager::Find(int32 roomId)
{
    auto it = _rooms.find(roomId);
    return (it != _rooms.end()) ? it->second : nullptr;
}

const PacketSessionRef& RoomManager::FindToSession(int32 roomId) const
{
    auto it = _roomIdToServerSession.find(roomId);
    return (it != _roomIdToServerSession.end()) ? it->second : nullptr;
}



void RoomManager::FindToRoomServerInfo_Connect(const int32 roomId, const int32& sessionId)
{
    //Redis에서 방 정보 json에서 ip port를 찾아서 멤버변수 _roomIdToServerSession 에 넣어준다.
    //물론 기존에 없던 roomserver정보면 연결시켜주고 넣어줘야겟지??
    std::string redisKey = "room:" + std::to_string(roomId);
    std::string query = "GET " + redisKey;

    auto onRoomInfoReceived = [&sessionId](void* reply)
    {
        redisReply* r = static_cast<redisReply*>(reply);
        if (!r || r->type != REDIS_REPLY_STRING)
        {
            //방 정보가 redis에도 없으므로 클라이언트는 존재하지 않는 방번호에 접근할려고 하는 것
        }

        std::string json_str = r->str;
        auto json = nlohmann::json::parse(json_str);

        std::wstring ip = Utils::Utf8ToWstring(json["ip"]);
        int16 port = json["port"];
        int32 roomId = json["id"];

        //해당 방정보를 가지고있는 RoomServer에게 연결 요청 연결이 완료되면 비동기로 세션의 OnCnnected()호출하기 때문에 OnConnect()에서 마무리 작업
        //if (GPClientService->AddRoomServerConnection(roomId,ip, port, sessionId));
    };

  /*  RedisUtils::RAsyncCommandCallback(
        GRedisConnection->GetContext(),
        onRoomInfoReceived,
        query.c_str()
    );*/
}

void RoomManager::Add_RoomIdToServerSession(const int32& roomId, const PacketSessionRef& session)
{
    WRITE_LOCK;
    {
        _roomIdToServerSession[roomId] = session;
    }
}

void RoomManager::DoRoomUpdate()
{

}

