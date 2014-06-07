#pragma once
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "BasePacket.h"

class SFAvroPacketImpl : public BasePacket
{
public:
	SFAvroPacketImpl();
	~SFAvroPacketImpl();

	SFPacketHeader* GetHeader() { return &m_packetHeader; }
	virtual bool Decode(char* pBuf, unsigned int nSize) = 0;
	virtual bool Encode(avro::EncoderPtr e) = 0;

	virtual void* GetBuffer() = 0;
	virtual int GetBufferSize() = 0;

private:
	SFPacketHeader m_packetHeader;
};

