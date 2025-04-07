#include "pch.h"
#include "ServerPacketHandler.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}
