#pragma once
#include "BasePacket.h"
#include "SFPacketIOBuffer.h"
#include "SFAvroProtocol.h"

class AvroClientProtocol : public SFAvroProtocol
{
public:
	AvroClientProtocol();
	virtual ~AvroClientProtocol();

	virtual BasePacket* CreateIncomingPacketFromPacketId(int packetId) override;
};
