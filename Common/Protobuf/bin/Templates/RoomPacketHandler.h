#pragma once
#include "ServerProtocol.pb.h"

using ServerPacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern ServerPacketHandlerFunc GServerPacketHandler[UINT16_MAX];

enum class ServerPacketType : uint16
{
{%- for key, value in parser.msgid_enum.items() %}
    {{ key.upper() }} = {{ value }},
{%- endfor %}
};

{%- for pkt in parser.recv_pkt %}
bool Handle_{{pkt.name}}(PacketSessionRef& session, ServerProtocol::{{pkt.name}}& pkt);
{%- endfor %}

class {{output}}
{
public:
	static void Init()
	{
		//for (int32 i = 0; i < UINT16_MAX; i++)
             //GServerPacketHandler[i] = Handle_INVALID;

{%- for pkt in parser.recv_pkt %}
		GServerPacketHandler[static_cast<uint16>(ServerPacketType::PKT_{{pkt.name}})] = [](PacketSessionRef& session, BYTE* buffer, int32 len) { return HandlePacket<ServerProtocol::{{pkt.name}}>(Handle_{{pkt.name}}, session, buffer, len); };
{%- endfor %}
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
        SendPacketHeader* header = reinterpret_cast<SendPacketHeader*>(buffer);
		return GServerPacketHandler[header->id](session, buffer, len);
	}

{%- for pkt in parser.send_pkt %}
	static SendBufferRef MakeSendBuffer(ServerProtocol::{{pkt.name}}& pkt) { return MakeSendBuffer(pkt, ServerPacketType::PKT_{{pkt.name}}); }
{%- endfor %}

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
