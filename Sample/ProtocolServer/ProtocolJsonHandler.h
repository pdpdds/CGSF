#pragma once
class ProtocolJsonHandler
{
public:
	ProtocolJsonHandler();
	virtual ~ProtocolJsonHandler();

	bool OnPacketSample1(BasePacket* pPacket);
	bool OnPacketSample2(BasePacket* pPacket);
	bool OnPacketSample3(BasePacket* pPacket);
	bool OnPacketSample4(BasePacket* pPacket);

};

