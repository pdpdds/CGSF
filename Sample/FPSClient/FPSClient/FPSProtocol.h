#pragma once
#include <wtypes.h>
#include "EngineInterface/EngineStructure.h"
#include "SFCGProtobufClientProtocol.h"

class FPSProtocol : public SFCGProtobufClientProtocol
{
public:
	FPSProtocol(void);
	virtual ~FPSProtocol(void);

	BasePacket* CreateIncomingPacketFromPacketId(int packetId) override;
};

