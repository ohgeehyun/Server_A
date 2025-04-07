#pragma once
#include "ServerProtocol.pb.h"

using ServerPacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

enum : uint16
{
    PKT_EMPTY_VALUE = 0,
    PKT_S_MESSAGE_GAME = 1001,
    PKT_C_MESSAGE_GAME = 1002,
};

// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_MESSAGE_GAME(PacketSessionRef& session, ServerProtocol::C_MESSAGE_GAME& pkt);

class RoomPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
             GServerPacketHandler[i] = Handle_INVALID;
		GServerPacketHandler[PKT_C_MESSAGE_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::C_MESSAGE_GAME>(Handle_C_MESSAGE_GAME, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);
		return GServerPacketHandler[header->id](session, buffer, len);
	}

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