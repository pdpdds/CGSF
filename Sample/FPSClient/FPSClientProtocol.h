#pragma once
#include "SFProtobufProtocol.h"

class FPSClientProtocol : public SFProtobufProtocol
{
public:
	FPSClientProtocol(void);
	virtual ~FPSClientProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

