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
	bool SendRequest(BasePacket* pPacket);

	bool DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();

protected:

private:
	JsonBuilder m_builder;

	bool GetCompleteNode(SFJsonPacket* pPacket);
};

