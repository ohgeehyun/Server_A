#pragma once
class UserServerSession : public PacketSession
{
public:
    virtual void OnConnected() override;
    virtual void OnDisConnected() override;
    virtual void OnRecvPacket(BYTE* buffer, int32 len);

private:
};

