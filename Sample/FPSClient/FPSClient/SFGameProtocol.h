#pragma once
#include <wtypes.h>
#include "SFProtobufProtocol.h"

class SFGameProtocol : public SFProtobufProtocol
{
public:
	SFGameProtocol(void);
	virtual ~SFGameProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

