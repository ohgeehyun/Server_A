#include "pch.h"
#include "ServerSession.h"
#include "ServerSessionManager.h"
#include "ServerPacketHandler.h""
#include "RoomManager.h"


ServerSessionManager* GServerSessionManager;

void ServerSession::OnConnected()
{
    GServerSessionManager->Add(static_pointer_cast<ServerSession>(shared_from_this()));
    wcout << " RoomServer " << this->GetAddress().GetIpAdress() + L" : " << this->GetAddress().GetPort() << " 연결 완료 " << endl;
}

void ServerSession::OnDisConnected()
{
    cout <<" RoomServer 연결 종료 " << endl;
}

void ServerSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);

    ServerPacketHandler::HandlePacket(session, buffer, len);
}

void ServerSession::OnConnected(const int32 roomid, const int32 callOwnerId)
{

}
