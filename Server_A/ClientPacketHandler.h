#pragma once
#include "Protocol.pb.h"
#include "GameSession.h"
using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
    PKT_BLOCK_VALUE = 0,
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
    PKT_S_VERIFY = 1011,
    PKT_C_VERIFY = 1012,
    PKT_C_CREATE_ROOM = 1013,
    PKT_C_ROOM_LIST = 1014,
    PKT_S_CREATE_ROOM = 1015,
    PKT_C_ENTER_GAME = 1016,
    PKT_S_MESSAGE = 1017,
    PKT_C_MESSAGE = 1018,
    PKT_C_LEAVE_GAME = 1019,
    PKT_S_EXIT_GAME = 1020,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt);
bool Handle_C_CREATE_ROOM(PacketSessionRef& session, Protocol::C_CREATE_ROOM& pkt);
bool Handle_C_ROOM_LIST(PacketSessionRef& session, Protocol::C_ROOM_LIST& pkt);
bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_SKILL(PacketSessionRef& session, Protocol::C_SKILL& pkt);
bool Handle_C_VERIFY(PacketSessionRef& session, Protocol::C_VERIFY& pkt);
bool Handle_C_MESSAGE(PacketSessionRef& session, Protocol::C_MESSAGE& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_ENTER_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ENTER_GAME>(Handle_C_ENTER_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_CREATE_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_CREATE_ROOM>(Handle_C_CREATE_ROOM, session, buffer, len); };
		GPacketHandler[PKT_C_ROOM_LIST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_ROOM_LIST>(Handle_C_ROOM_LIST, session, buffer, len); };
		GPacketHandler[PKT_C_LEAVE_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_LEAVE_GAME>(Handle_C_LEAVE_GAME, session, buffer, len); };
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len); };
		GPacketHandler[PKT_C_SKILL] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_SKILL>(Handle_C_SKILL, session, buffer, len); };
		GPacketHandler[PKT_C_VERIFY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_VERIFY>(Handle_C_VERIFY, session, buffer, len); };
		GPacketHandler[PKT_C_MESSAGE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<Protocol::C_MESSAGE>(Handle_C_MESSAGE, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, PKT_S_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CREATE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_CREATE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LEAVE_GAME& pkt) { return MakeSendBuffer(pkt, PKT_S_LEAVE_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::S_EXIT_GAME& pkt) { return MakeSendBuffer(pkt, PKT_S_EXIT_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::S_SPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_SPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_DESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_SKILL& pkt) { return MakeSendBuffer(pkt, PKT_S_SKILL); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHANGEHP& pkt) { return MakeSendBuffer(pkt, PKT_S_CHANGEHP); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DIE& pkt) { return MakeSendBuffer(pkt, PKT_S_DIE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_VERIFY& pkt) { return MakeSendBuffer(pkt, PKT_S_VERIFY); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MESSAGE& pkt) { return MakeSendBuffer(pkt, PKT_S_MESSAGE); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        PacketType pkt;
        RecvPacketHeader* header = reinterpret_cast<RecvPacketHeader*>(buffer);
        if (pkt.ParseFromArray(buffer + sizeof(RecvPacketHeader) + header->jwtsize, len - sizeof(RecvPacketHeader) - header->jwtsize) != true)
            return false;

        BYTE* jwtToken = buffer + sizeof(RecvPacketHeader);
        std::string jwtTokenStr(reinterpret_cast<char*>(jwtToken), header->jwtsize);

        if (dynamic_pointer_cast<GameSession>(session)->GetIsJwtVerify())
            if (!JwtUtils::GJwtVerify(jwtTokenStr, ""))
                return false;

        return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(SendPacketHeader);

		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
		SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		ASSERT_CRASH(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};