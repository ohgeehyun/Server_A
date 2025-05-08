#pragma once

class RoomManager : public JobQueue 
{
public:

    RoomRef Add(const ServerProtocol::C_CREATE_ROOM pkt,int32 roomId, PacketSessionRef& session);
    bool    Remove(int32 roomId);
    const RoomRef& Find(int32 roomId) const;
    RoomManagerRef GetSharedPtr() { return static_pointer_cast<RoomManager>(shared_from_this()); }

    //redis에서 방 번호를 받아온 뒤 방 생성
    void RoomidToRedis_CreateRoom(const ServerProtocol::C_CREATE_ROOM& pkt, PacketSessionRef& session);
    //방 생성 완료 후 방 생성 요청을한 UserServer에게 response
    void ResponseCreateRoomPacket(RoomRef& room, PacketSessionRef& session,int32 clientSessionId);

    PlayerRef GetUserInRoom(const int32& roomid,const string& userid);

    void DoRoomUpdate();


private:

    USE_LOCK;
    HashMap<int32, RoomRef> _rooms;
    int32 _roomid = 1;
};