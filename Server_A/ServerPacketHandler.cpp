#include "pch.h"
#include "ServerPacketHandler.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}

bool Handle_S_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::S_CREATE_ROOM& pkt)
{
    return false;
}
