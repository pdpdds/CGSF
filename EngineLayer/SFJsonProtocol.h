#pragma once
#include "JsonBuilder.h"

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

	BOOL DisposePacket(SFJsonPacket* pPacket);

protected:
	SFJsonPacket* CreatePacket();

private:
	JsonBuilder m_builder;
};

