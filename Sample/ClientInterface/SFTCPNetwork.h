#pragma once
#include "GoogleLog.h"
#include "SFMacro.h"
#include "SFEngine.h"

class INetworkCallback;

class SFTCPNetwork
{
public:
	SFTCPNetwork(void);
	virtual ~SFTCPNetwork(void);

	BOOL Initialize(char* szModuleName, INetworkCallback* pTCPCallBack);
	BOOL Start(char* szIP, unsigned short Port);
	BOOL Update();
	BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize );

	SFSYSTEM* GetNetwork();

protected:

private:
	SFSYSTEM* m_TCPClient; // TCP
	INetworkCallback* m_pTCPCallBack;
};

