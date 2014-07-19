#pragma once
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"
#include "SFBasePacketProtocol.h"

class SFAvroPPacket;

class SFAvroProtocol : public SFBasePacketProtocol
{
public:
	SFAvroProtocol();
	virtual ~SFAvroProtocol();

	bool Initialize(int ioBufferSize, unsigned short packetSize, int packetOption);

	bool Reset();
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	BasePacket* GetPacket(int& errorCode);

	bool SendRequest(BasePacket* pPacket);

	
	bool DisposePacket(BasePacket* pPacket);
	//BasePacket* CreatePacket();

	virtual BasePacket* CreateIncomingPacketFromPacketId(int PacketId) = 0;

protected:

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};
