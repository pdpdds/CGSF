#pragma once
#include "SFIOBuffer.h"

class SFPacketIOBuffer : public SFIOBuffer
{

public:
	SFPacketIOBuffer(void);
	virtual ~SFPacketIOBuffer(void);

	BOOL GetPacket(SFPacket* pPacket, int& ErrorCode);
};
