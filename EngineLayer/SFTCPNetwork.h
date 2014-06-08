#pragma once
#pragma warning(disable : 4996)
//#include "GoogleLog.h"
#include "SFMacro.h"
#include "SFEngine.h"
#include "liblfds611.h"

class INetworkCallback;

class SFTCPNetwork
{
public:
	SFTCPNetwork(void);
	virtual ~SFTCPNetwork(void);

	BOOL Initialize(char* szModuleName, INetworkCallback* pTCPCallBack);
	BOOL Start(char* szIP, unsigned short Port);
	BOOL Update();
	BOOL SendRequest(BasePacket* pPacket);

	SFEngine* GetNetwork();

	void SetLogicDispatcher(ILogicDispatcher* pDispatcher);

	void SetPacketProtocol(IPacketProtocol* pProtocol);

	bool IsConnected();

	BasePacket* GetRPCResult();

protected:

private:
	SFEngine* m_TCPClient; // TCP
	INetworkCallback* m_pTCPCallBack;
	struct lfds611_queue_state* m_pQueue;
};

