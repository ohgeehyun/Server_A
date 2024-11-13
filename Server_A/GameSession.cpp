#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include <iomanip>
#include "ClientPacketHandler.h"

GameSessionManager* SessionManager;

void GameSession::OnConnected()
{
    SessionManager->Add(static_pointer_cast<GameSession>(shared_from_this()));
    cout << "접속 인원 추가" << endl;
    Protocol::S_CHAT Chat;
    string text = u8"안녕하세요. 클라이언트";

    Chat.set_context(text);
    auto sendBuffer = ClientPacketHandler::MakeSendBuffer(Chat);
    SessionManager->Broadcast(sendBuffer);
}

void GameSession::OnDisConnected()
{
    SessionManager->Remove(static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    PacketSessionRef session = GetPacketSessionRef();
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

    //TODO : 클라이언트 패킷 핸들러 작성
    ClientPacketHandler::HandlePacket(session, buffer, len);
}