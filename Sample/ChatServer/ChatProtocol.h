#pragma once
#include "SFProtobufProtocol.h"

class ChatProtocol : public SFProtobufProtocol
{
public:
	ChatProtocol(void);
	virtual ~ChatProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) override;

};

