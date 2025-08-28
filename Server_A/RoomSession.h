#pragma once

class RoomSession : public PacketSession
{
public:
    virtual void OnConnected() override;
    virtual void OnDisConnected() override;
    virtual void OnRecvPacket(BYTE* buffer, int32 len);

    void OnConnected(const int32 roomid,const int32 callOwnerId);

    RoomSessionRef GetServerSessionRef() { return static_pointer_cast<RoomSession>(shared_from_this());}

private:
};

