#include "pch.h"
#include "RoomPacketHandler.h"
#include "Room.h"
#include "RoomManager.h"

ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}
bool Handle_C_ENTER_GAME(PacketSessionRef& session, ServerProtocol::C_ENTER_GAME& pkt)
{
    return false;
}
bool Handle_C_MOVE(PacketSessionRef& session, ServerProtocol::C_MOVE& pkt)
{
    return false;
}
bool Handle_C_SKILL(PacketSessionRef& session, ServerProtocol::C_SKILL& pkt)
{
    return false;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::C_CREATE_ROOM& pkt)
{
    //방 이름 검사
    if (pkt.roomname() == "")
        return false;

    // Redis 비동기 요청 + 콜백 정의
   // RoomManager::GetInstance().RequestCreateRoomFromRedis(pkt, UserSession);

    return true;
}

bool Handle_C_GET_ROOMINFO(PacketSessionRef& session, ServerProtocol::C_GET_ROOMINFO& pkt)
{

    const RoomRef& room = RoomManager::GetInstance().Find(pkt.roomid());

    if (pkt.roomid() <= 0)
        return false;

    if (room == nullptr)
        return false;

    ServerProtocol::S_GET_ROOMINFO packet;
    packet.set_roomid(room->GetRoomId());
    packet.set_roomname(room->GetRoomName());
    packet.set_rootuser(room->GetRootUser());
    packet.set_pwdyn(room->GetPwdYn());
    packet.set_roompwd(room->GetRoomPwd());
    packet.set_callownersessionid(pkt.owner());
    auto Packet = RoomPacketHandler::MakeSendBuffer(packet);
    session->Send(Packet);


    return true;
}
