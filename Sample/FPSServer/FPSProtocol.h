#pragma once
#include "SFCGProtobufProtocol.h"

class FPSProtocol : public SFCGProtobufProtocol
{
public:
	FPSProtocol(void);
	virtual ~FPSProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId(int packetId) override;
};

