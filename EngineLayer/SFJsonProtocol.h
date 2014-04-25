#pragma once
#include <Json/JsonBuilder.h>

class ISession;
class BasePacket;
class SFJsonPacket;

class SFJsonProtocol
{
public:
	SFJsonProtocol(void);
	virtual ~SFJsonProtocol(void);

	bool Initialize(int ioBufferSize = PACKET_DEFAULT_IO_SIZE, USHORT packetDataSize = PACKET_DEFAULT_PACKET_SIZE);

	bool Reset();
	BasePacket* GetPacket(int& errorCode);
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool SendRequest(BasePacket* pPacket);

	bool DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();
	bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize);

protected:

private:
	JsonBuilder m_builder;

	bool GetCompleteNode(SFJsonPacket* pPacket);
};

