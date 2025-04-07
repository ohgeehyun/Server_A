#include "pch.h"
#include "RoomPacketHandler.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}

bool Handle_C_MESSAGE_GAME(PacketSessionRef& session, ServerProtocol::C_MESSAGE_GAME& pkt)
{
    cout << pkt.message() << endl;
    return true;
}