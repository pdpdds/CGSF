#pragma once
#include "SFProtobufProtocol.h"

class ProtocolClientProtocol : public SFProtobufProtocol
{
public:
	ProtocolClientProtocol(void);
	virtual ~ProtocolClientProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId(int PacketId) override;
};