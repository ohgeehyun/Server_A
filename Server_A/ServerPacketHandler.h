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

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_ENTER_GAME(PacketSessionRef& session, ServerProtocol::S_ENTER_GAME& pkt);
bool Handle_S_LEAVE_GAME(PacketSessionRef& session, ServerProtocol::S_LEAVE_GAME& pkt);
bool Handle_S_EXIT_GAME(PacketSessionRef& session, ServerProtocol::S_EXIT_GAME& pkt);
bool Handle_S_SPAWN(PacketSessionRef& session, ServerProtocol::S_SPAWN& pkt);
bool Handle_S_DESPAWN(PacketSessionRef& session, ServerProtocol::S_DESPAWN& pkt);
bool Handle_S_MOVE(PacketSessionRef& session, ServerProtocol::S_MOVE& pkt);
bool Handle_S_CREATE_ROOM(PacketSessionRef& session, ServerProtocol::S_CREATE_ROOM& pkt);
bool Handle_S_SKILL(PacketSessionRef& session, ServerProtocol::S_SKILL& pkt);
bool Handle_S_MESSAGE(PacketSessionRef& session, ServerProtocol::S_MESSAGE& pkt);
bool Handle_S_CHANGEHP(PacketSessionRef& session, ServerProtocol::S_CHANGEHP& pkt);
bool Handle_S_DIE(PacketSessionRef& session, ServerProtocol::S_DIE& pkt);
bool Handle_S_GET_ROOMINFO(PacketSessionRef& session, ServerProtocol::S_GET_ROOMINFO& pkt);

class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
             GServerPacketHandler[i] = Handle_INVALID;
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_ENTER_GAME)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_ENTER_GAME>(Handle_S_ENTER_GAME, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_LEAVE_GAME)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_LEAVE_GAME>(Handle_S_LEAVE_GAME, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_EXIT_GAME)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_EXIT_GAME>(Handle_S_EXIT_GAME, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_SPAWN)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_SPAWN>(Handle_S_SPAWN, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_DESPAWN)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_DESPAWN>(Handle_S_DESPAWN, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_MOVE)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_MOVE>(Handle_S_MOVE, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_CREATE_ROOM)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_CREATE_ROOM>(Handle_S_CREATE_ROOM, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_SKILL)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_SKILL>(Handle_S_SKILL, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_MESSAGE)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_MESSAGE>(Handle_S_MESSAGE, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_CHANGEHP)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_CHANGEHP>(Handle_S_CHANGEHP, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_DIE)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_DIE>(Handle_S_DIE, session, buffer, len); };
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_S_GET_ROOMINFO)] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::S_GET_ROOMINFO>(Handle_S_GET_ROOMINFO, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);
		return GServerPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(ServerProtocol::C_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_C_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::C_SKILL& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_C_SKILL); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::C_CREATE_ROOM& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_C_CREATE_ROOM); }
	static SendBufferRef MakeSendBuffer(ServerProtocol::C_GET_ROOMINFO& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_C_GET_ROOMINFO); }

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