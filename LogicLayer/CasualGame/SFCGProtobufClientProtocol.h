#pragma once
#include "SFProtobufProtocol.h"

class SFCGProtobufClientProtocol : public SFProtobufProtocol
{
public:
	SFCGProtobufClientProtocol(void);
	virtual ~SFCGProtobufClientProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId(int packetId) override;
};

