#pragma once
#include <Json/JsonBuilder.h>
#include "SFBasePacketProtocol.h"

class ISession;
class BasePacket;
class SFJsonPacket;

class SFJsonProtocol : public SFBasePacketProtocol
{
public:
	SFJsonProtocol(void);
	virtual ~SFJsonProtocol(void);

	bool Initialize(int ioBufferSize, unsigned short packetSize, int packetOption);

	bool Reset();
	BasePacket* GetPacket(int& errorCode);
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize);

	static bool DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();

protected:

private:
	JsonBuilder m_builder;
	char* m_pBuffer;
	bool GetCompleteNode(SFJsonPacket* pPacket);
};

