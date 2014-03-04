#pragma once
#include "SFProtobufProtocol.h"

class SFCGProtobufProtocol : public SFProtobufProtocol
{
public:
	SFCGProtobufProtocol(void);
	virtual ~SFCGProtobufProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

