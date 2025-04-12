#include "pch.h"
#include "RoomPacketHandler.h"
#include "RoomManager.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::C_CREATE_ROOM& pkt)
{
    UserServerSessionRef UserSession = static_pointer_cast<UserServerSession>(session);

    //방 이름 검사
    if (pkt.roomname() == "")
        return false;

    // Redis 비동기 요청 + 콜백 정의
    RoomManager::GetInstance().RequestCreateRoomFromRedis(pkt, UserSession);

    return false;
}
