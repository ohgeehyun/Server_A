#include "pch.h"
#include "UserServerSession.h"
#include "SessionManager.h"

SessionManager* UserServerSessionManager;

void UserServerSession::OnConnected()
{
    UserServerSessionManager->Add(static_pointer_cast<UserServerSession>(shared_from_this()));
    cout << "RoomServer : Session Add Success " << endl;
}

void UserServerSession::OnDisConnected()
{
    //세션 연결 끊어질 때 호출 될 함수
}

void UserServerSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);


   //패킷 처리 함수 
   // ClientPacketHandler::HandlePacket(session, buffer, len);
}
