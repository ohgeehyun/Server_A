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

bool Handle_S_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::S_CREATE_ROOM& pkt)
{

    //RoomServer에서 방만들기 실패 하였음. 혹여나 실패에 따른 분기가 필요할시 생성
    if (pkt.result() == false)
        return;

    GameSessionRef ClientSession = GGameSessionManager->Find(pkt.sessionid());

    if (ClientSession == nullptr)
        return false;
    

    //UserRoomServer에서도 일단은 간단한 Room정보는 가지고 있는 방향
    RoomRef room = RoomManager::GetInstance().Add(pkt);
    
    //클라이언트에게 결과전달
    Protocol::S_CREATE_ROOM resultPacket;
    if (room != nullptr)
    {
        resultPacket.set_result(true);
        resultPacket.set_roomid(pkt.roomid());
    }
    else
    {
        resultPacket.set_result(false);
    }

    auto resultPacketBuffer = ClientPacketHandler::MakeSendBuffer(resultPacket);
    ClientSession->Send(resultPacketBuffer);

    return true;
}
