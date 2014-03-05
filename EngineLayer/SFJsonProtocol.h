#pragma once
#include <json/JsonBuilder.h>

class ISession;
class BasePacket;
class SFJsonPacket;

class SFJsonProtocol
{
public:
	SFJsonProtocol(void);
	virtual ~SFJsonProtocol(void);

	BOOL Reset();
	BasePacket* GetPacket(int& ErrorCode);
	BOOL AddTransferredData(char* pBuffer, DWORD dwTransferred);
	BOOL SendRequest(ISession* pSession, BasePacket* pPacket);

	BOOL DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();
	bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize);

protected:

private:
	JsonBuilder m_builder;
};

