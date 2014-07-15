#pragma once
#include "SFAvroProtocol.h"

class ProtocolAvroProtocol : public SFAvroProtocol
{
public:
	ProtocolAvroProtocol();
	virtual ~ProtocolAvroProtocol();

	virtual BasePacket* CreateIncomingPacketFromPacketId(int packetId) override;
};

