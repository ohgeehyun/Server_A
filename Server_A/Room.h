#pragma once
#include "Protocol.pb.h"

class Room : public enable_shared_from_this<Room>
{
public:
    Room();
    ~Room();

    void EnterGame(PlayerRef& newPlayer);
    void LeaveGame(int PlayerId);
    void Broadcast(SendBufferRef sendBuffer);
    void HandleMove(PlayerRef& player, Protocol::C_MOVE& pkt);
    void HandleSkill(PlayerRef& player, Protocol::C_SKILL& pkt);

    int32 GetRoomId() { return _roomId; }
    void SetRoomId(int32 RoomId) { _roomId = RoomId; }


private:
    int32 _roomId;
    USE_LOCK;
    List<PlayerRef> _players;
};