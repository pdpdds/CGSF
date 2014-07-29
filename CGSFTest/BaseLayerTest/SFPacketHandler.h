#pragma once
#include "EngineInterface/EngineStructure.h"
#include "SFProtobufPacket.h"
#include "PacketCore.pb.h"

void sampleFunc(BasePacket* packet);

class SFPacketHandler
{
public:
	SFPacketHandler(void);
	virtual ~SFPacketHandler(void);

	void OnAuth(BasePacket* packet);
};
