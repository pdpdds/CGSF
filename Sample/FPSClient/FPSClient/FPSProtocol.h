#pragma once
#include <wtypes.h>
#include "SFCGProtobufClientProtocol.h"

class FPSProtocol : public SFCGProtobufClientProtocol
{
public:
	FPSProtocol(void);
	virtual ~FPSProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

