#pragma once
#include "SFIOBuffer.h"
#include "SFStructure.h"
class SFPacket;

class SFPacketIOBuffer : public SFIOBuffer
{

public:
	SFPacketIOBuffer(void);
	virtual ~SFPacketIOBuffer(void);

	bool GetPacket(SFPacketHeader& header, char* pBuffer, unsigned short packetSize, int& errorCode);
	bool GetHeader(SFPacketHeader& header, int& errorCode);
};
