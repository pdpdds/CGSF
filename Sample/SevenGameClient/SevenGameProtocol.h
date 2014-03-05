#pragma once
#include "CasualGameProtocol.h"

class SevenGameProtocol : public CasualGameProtocol
{
public:
	SevenGameProtocol(void);
	virtual ~SevenGameProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};


