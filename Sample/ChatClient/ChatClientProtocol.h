#pragma once
#include "SFProtobufProtocol.h"

class ChatClientProtocol : public SFProtobufProtocol
{
public:
	ChatClientProtocol(void);
	virtual ~ChatClientProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;
};
