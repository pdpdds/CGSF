#pragma once
#include "SFCGProtobufClientProtocol.h"

class SevenGameProtocol : public SFCGProtobufClientProtocol
{
public:
	SevenGameProtocol(void);
	virtual ~SevenGameProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int packetId ) override;
};


