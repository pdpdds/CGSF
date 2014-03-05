#pragma once
#include <wtypes.h>
#include "CasualGameProtocol.h"

class SFGameProtocol : public CasualGameProtocol
{
public:
	SFGameProtocol(void);
	virtual ~SFGameProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

