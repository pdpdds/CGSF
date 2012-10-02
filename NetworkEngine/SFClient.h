#pragma once

class INetworkFramework;
class INetworkCallback;
class ILogicEntry;

class SFClient
{
public:
	SFClient(void);
	virtual ~SFClient(void);

	BOOL Run(INetworkFramework* pFramework, ILogicEntry* pLogic);

	void SetNetworkCallback(INetworkCallback* pNetworkCallback){m_pNetworkCallback = pNetworkCallback;}
	BOOL Update();
	bool Send( USHORT PacketID, char* pMessage, int BufSize );

protected:
	

private:
	INetworkCallback* m_pNetworkCallback;
	INetworkFramework* m_pNetworkFramework;
};