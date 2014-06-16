#pragma once
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

class SFAvroPPacket;

class SFAvroProtocol
{
public:
	SFAvroProtocol();
	virtual ~SFAvroProtocol();

	bool Initialize(int ioBufferSize, USHORT packetDataSize);

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
