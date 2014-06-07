#pragma once
class ProtocolAvroHandler
{
public:
	ProtocolAvroHandler();
	virtual ~ProtocolAvroHandler();

	bool OnPacketSample1(BasePacket* pPacket);
	bool OnPacketSample2(BasePacket* pPacket);
	bool OnPacketSample3(BasePacket* pPacket);
	bool OnPacketSample4(BasePacket* pPacket);
};

