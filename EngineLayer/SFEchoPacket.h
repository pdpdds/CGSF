#pragma once
#include "BasePacket.h"
#include "DataBuffer.h"

class SFEchoPacket : public BasePacket
{
public:
	SFEchoPacket();
	virtual ~SFEchoPacket();

	DataBuffer m_Buffer;
};

