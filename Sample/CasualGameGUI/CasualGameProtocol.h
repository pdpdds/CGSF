#pragma once
#include "SFProtobufProtocol.h"

class CasualGameProtocol : public SFProtobufProtocol
{
public:
	CasualGameProtocol(void);
	virtual ~CasualGameProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

