#include "pch.h"
#include "ServerSession.h"
#include "ServerSessionManager.h"
#include "ServerPacketHandler.h"


ServerSessionManager* GServerSessionManager;

void ServerSession::OnConnected()
{
    GServerSessionManager->Add(static_pointer_cast<ServerSession>(shared_from_this()));
    cout << " RoomServer 연결 완료 " << endl;

    ServerProtocol::C_MESSAGE_GAME chat;
    string message = "hi room server";
    chat.set_message(message);
    auto messagebuffer = ServerPacketHandler::MakeSendBuffer(chat);
    Send(messagebuffer);
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
