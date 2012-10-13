#pragma once

class INetworkFramework;
class INetworkCallback;
class ILogicEntry;

class SFPacket;

class SFClient
{
public:
	SFClient(void);
	virtual ~SFClient(void);

	BOOL Run(INetworkFramework* pFramework, ILogicEntry* pLogic);
	BOOL Stop();

	void SetNetworkCallback(INetworkCallback* pNetworkCallback){m_pNetworkCallback = pNetworkCallback;}
	BOOL Update();

	bool Send(int Serial, USHORT PacketID, char* pMessage, int BufSize );
	BOOL Send(int Serial, SFPacket* pPacket);

protected:
	

private:
	INetworkCallback* m_pNetworkCallback;
	INetworkFramework* m_pNetworkFramework;
};