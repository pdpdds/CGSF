#pragma once
#include "SFClient.h"
#include "GoogleLog.h"
#include "SFMacro.h"
#include "SFACEFramework.h"
#include "SFEngine.h"

class SFTCPNetwork
{
public:
	SFTCPNetwork(void);
	virtual ~SFTCPNetwork(void);

	BOOL Initialize(INetworkCallback* pTCPCallBack);
	BOOL Run();
	BOOL Update();
	BOOL Send( USHORT PacketID, char* pMessage, int BufSize );

	SFSYSTEM_CLIENT* GetNetwork();

protected:

private:
	SFSYSTEM_CLIENT* m_TCPClient; // TCP
};

