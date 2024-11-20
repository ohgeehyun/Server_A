#include "pch.h"
#include "ClientPacketHandler.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "Player.h"
#include <codecvt> 
PacketHandlerFunc GPacketHandler[UINT16_MAX];



std::wstring Utf8ToUtf16(const std::string& utf8Str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(utf8Str);
}

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
    // TODO : Log
    return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
    
    cout << "C_MOVE : " << pkt.posinfo().posx() <<","<< pkt.posinfo().posy()<< endl;
    
    //바로 if문에서 player정보를 체크해도 되지만 멀티스레드환경에서 과연 안전 하지 못 하다.
    PlayerRef player = gameSession->GetPlayer();
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    //TODO : 원래라면 클라이언트의 신용을 검증할 필요 가 있음. 나중에 기반 마련되면 수정
    
    room->HandleMove(player,pkt);
   
    return true;
}

bool Handle_C_SKILL(PacketSessionRef& session, Protocol::C_SKILL& pkt)
{
    GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

    PlayerRef player = gameSession->GetPlayer();
    if (player == nullptr)
        return false;

    RoomRef room = player->GetRoom();
    if (room == nullptr)
        return false;

    room->HandleSkill(player, pkt);

    return true;
}


