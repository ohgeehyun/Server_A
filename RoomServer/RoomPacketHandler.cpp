#include "pch.h"
#include "RoomPacketHandler.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::C_CREATE_ROOM& pkt)
{
    return false;
}
