#pragma once
#include "SFProtocol.h"
#include "DataBuffer.h"

class ISession;
class BasePacket;

class SFProtobufProtocol : public SFProtocol
{
public:
	SFProtobufProtocol(void);
	virtual ~SFProtobufProtocol(void);

	BOOL Reset();
	BasePacket* GetPacket(int& ErrorCode);
	BOOL AddTransferredData(char* pBuffer, DWORD dwTransferred);
	//BOOL SendRequest(ISession* pSession, BasePacket* pPacket);

	bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize);

	virtual void disposeOutgoingPacket(BasePacket* pPacket);
	virtual void disposeIncomingPacket(BasePacket* pPacket);

protected:
	virtual BasePacket* CreateIncomingPacketFromPacketId( int PacketId ) = 0;

protected:
	virtual int encodeOutgoingPacket(BasePacket& packet);
	virtual int frameOutgoingPacket(BasePacket& packet, DataBuffer& buffer, unsigned int& nWrittenBytes);

	//
	virtual int tryDeframeIncomingPacket( DataBuffer& Buffer,  BasePacket*& pPacket, int& serviceId, unsigned int& nExtractedBytes);
	virtual int decodeIncomingPacket(BasePacket* pPacket, int& PacketId);

protected:
	BOOL Initialize();

private:
	DataBuffer m_Buffer;
	DataBuffer oBuffer;
};

