#pragma once
#include "SFCGProtobufProtocol.h"

class SevenGameProtocol : public SFCGProtobufProtocol
{
public:
	SevenGameProtocol(void);
	virtual ~SevenGameProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

