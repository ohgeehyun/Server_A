#pragma once
#include "ServerProtocol.pb.h"

using ServerPacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

enum class ServerPacketType : uint16
{
    PKT_EMPTY_VALUE = 0,
    PKT_S_ENTER_GAME = 1001,
    PKT_S_LEAVE_GAME = 1002,
    PKT_S_SPAWN = 1003,
    PKT_S_DESPAWN = 1004,
    PKT_C_MOVE = 1005,
    PKT_S_MOVE = 1006,
    PKT_C_SKILL = 1007,
    PKT_S_SKILL = 1008,
    PKT_S_CHANGEHP = 1009,
    PKT_S_DIE = 1010,
    PKT_C_CREATE_ROOM = 1011,
    PKT_C_ROOM_LIST = 1012,
    PKT_S_CREATE_ROOM = 1013,
    PKT_C_ENTER_GAME = 1014,
    PKT_S_MESSAGE = 1015,
    PKT_C_MESSAGE = 1016,
    PKT_C_LEAVE_GAME = 1017,
    PKT_S_EXIT_GAME = 1018,
    PKT_S_GET_ROOMINFO = 1019,
    PKT_C_GET_ROOMINFO = 1020,
};
bool Handle_C_ENTER_GAME(PacketSessionRef& session, ServerProtocol::C_ENTER_GAME& pkt);
bool Handle_C_LEAVE_GAME(PacketSessionRef& session, ServerProtocol::C_LEAVE_GAME& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, ServerProtocol::C_MOVE& pkt);
bool Handle_C_SKILL(PacketSessionRef& session, ServerProtocol::C_SKILL& pkt);
bool Handle_C_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::C_CREATE_ROOM& pkt);
bool Handle_C_MESSAGE(PacketSessionRef& session, ServerProtocol::C_MESSAGE& pkt);

class RoomPacketHandler
{
public:
	static void Init()
	{
		//for (int32 i = 0; i < UINT16_MAX; i++)
             //GServerPacketHandler[i] = Handle_INVALID;
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_C_ENTER_GAME)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_ENTER_GAME>(Handle_C_ENTER_GAME, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_C_LEAVE_GAME)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_LEAVE_GAME>(Handle_C_LEAVE_GAME, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_C_MOVE)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_C_SKILL)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_SKILL>(Handle_C_SKILL, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_C_CREATE_ROOM)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_CREATE_ROOM>(Handle_C_CREATE_ROOM, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_C_MESSAGE)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_MESSAGE>(Handle_C_MESSAGE, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);
		return GServerPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_LEAVE_GAME& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_LEAVE_GAME); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_EXIT_GAME& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_EXIT_GAME); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_SPAWN& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_SPAWN); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_DESPAWN& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_DESPAWN); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_MOVE& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_MOVE); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_CREATE_ROOM& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_CREATE_ROOM); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_SKILL& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_SKILL); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_MESSAGE& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_MESSAGE); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_CHANGEHP& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_CHANGEHP); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::S_DIE& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_S_DIE); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        PacketType pkt;
        SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);
        if (pkt.ParseFromArray(buffer + sizeof(SendPacketHeader), len - sizeof(SendPacketHeader)) != true)
            return false;

        return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, ServerPacketType pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(SendPacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = static_cast<uint16>(pktId);
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};