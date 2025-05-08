#include "pch.h"
#include "RoomSession.h"
#include "RoomSessionManager.h"
#include "ServerPacketHandler.h""
#include "RoomManager.h"


RoomSessionManager* GRoomSessionManager;

void RoomSession::OnConnected()
{
    GRoomSessionManager->Add(static_pointer_cast<RoomSession>(shared_from_this()));
}

void RoomSession::OnDisConnected()
{
    cout <<" RoomServer 연결 종료 " << endl;
}

void RoomSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);

    ServerPacketHandler::HandlePacket(session, buffer, len);
}

void RoomSession::OnConnected(const int32 roomid, const int32 callOwnerId)
{

}
