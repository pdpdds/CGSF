#pragma once
#include "SFProtocol.h"
#include "DataBuffer.h"
#include "SFConstant.h"

class ISession;
class BasePacket;

class SFProtobufProtocol : public SFProtocol
{
public:
	SFProtobufProtocol(void);
	virtual ~SFProtobufProtocol(void);

	bool Initialize(int ioBufferSize, USHORT packetDataSize);

	bool Reset();
	BasePacket* GetPacket(int& ErrorCode);
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool SendRequest(BasePacket* pPacket);

	virtual void disposeOutgoingPacket(BasePacket* pPacket);
	virtual void disposeIncomingPacket(BasePacket* pPacket);
	BasePacket* CreatePacket(){ return NULL; }
	bool DisposePacket(BasePacket* pPacket);

protected:
	virtual BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) = 0;

protected:
	virtual int encodeOutgoingPacket(BasePacket& packet);
	virtual int frameOutgoingPacket(BasePacket& packet, DataBuffer& buffer, unsigned int& nWrittenBytes);

	//
	virtual int tryDeframeIncomingPacket( DataBuffer& Buffer,  BasePacket*& pPacket, int& serviceId, unsigned int& nExtractedBytes);
	virtual int decodeIncomingPacket(BasePacket* pPacket, int& PacketId);

protected:

private:
	DataBuffer m_Buffer;
	DataBuffer oBuffer;
};

