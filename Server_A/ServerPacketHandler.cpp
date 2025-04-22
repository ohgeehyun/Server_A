#include "pch.h"
#include "ServerPacketHandler.h"
#include "ClientPacketHandler.h"
#include "GameSessionManager.h"
#include "Room.h"
#include "RoomManager.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, ServerProtocol::S_ENTER_GAME& pkt)
{
    return false;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& session, ServerProtocol::S_LEAVE_GAME& pkt)
{
    return false;
}

bool Handle_S_EXIT_GAME(PacketSessionRef& session, ServerProtocol::S_EXIT_GAME& pkt)
{
    return false;
}

bool Handle_S_SPAWN(PacketSessionRef& session, ServerProtocol::S_SPAWN& pkt)
{
    return false;
}

bool Handle_S_DESPAWN(PacketSessionRef& session, ServerProtocol::S_DESPAWN& pkt)
{
    return false;
}

bool Handle_S_MOVE(PacketSessionRef& session, ServerProtocol::S_MOVE& pkt)
{
    return false;
}

bool Handle_S_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::S_CREATE_ROOM& pkt)
{
    return true;
}

bool Handle_S_SKILL(PacketSessionRef& session, ServerProtocol::S_SKILL& pkt)
{
    return false;
}

bool Handle_S_MESSAGE(PacketSessionRef& session, ServerProtocol::S_MESSAGE& pkt)
{
    return false;
}

bool Handle_S_CHANGEHP(PacketSessionRef& session, ServerProtocol::S_CHANGEHP& pkt)
{
    return false;
}

bool Handle_S_DIE(PacketSessionRef& session, ServerProtocol::S_DIE& pkt)
{
    return false;
}

bool Handle_S_GET_ROOMINFO(PacketSessionRef& session, ServerProtocol::S_GET_ROOMINFO& pkt)
{
    return false;
}
