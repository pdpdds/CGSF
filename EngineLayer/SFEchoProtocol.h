#pragma once

#include "SFBasePacketProtocol.h"
#include "DataBuffer.h"

class ISession;
class BasePacket;
class SFEchoPacket;

class SFEchoProtocol : public SFBasePacketProtocol
{
public:
	SFEchoProtocol(void);
	virtual ~SFEchoProtocol(void);

	bool Initialize(int ioBufferSize, unsigned short packetSize, int packetOption);

	bool Reset();
	BasePacket* GetPacket(int& errorCode);
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize);

	static bool DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();

	bool GetPacket(SFEchoPacket* pPacket);

protected:

private:	
	DataBuffer m_Buffer;
};

