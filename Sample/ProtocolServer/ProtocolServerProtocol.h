#pragma once
#include "SFProtobufProtocol.h"

class ProtocolServerProtocol : public SFProtobufProtocol
{
public:
	ProtocolServerProtocol(void);
	virtual ~ProtocolServerProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};

