#pragma once
#include "IUDPNetworkCallback.h"

typedef struct tag_PeerAddressInfo
{
	DWORD LocalIP;
	USHORT LocalPort;

	DWORD ExternalIP;
	USHORT ExternalPort;
}PeerAddressInfo;

class IP2PManager
{
public:
	IP2PManager(void){m_pUDPCallback = NULL;}
	virtual ~IP2PManager(void){}

	virtual BOOL Initialize(IUDPNetworkCallback* pCallback) = 0;
	virtual BOOL Finally() = 0;

	virtual BOOL RunP2P(char* szIP, unsigned short Port) = 0;
	virtual BOOL EndP2P() = 0;
	virtual BOOL Update() = 0;

	virtual BOOL AddPeer(PeerAddressInfo* pAddressInfo, BYTE& byIndex) = 0;
	virtual BOOL BroadCast(BYTE* pData, USHORT Size, BOOL ExceptMe = FALSE) = 0;
	virtual BOOL BroadCastWithRelayServer(BYTE* pData, USHORT Size) = 0;
	virtual BOOL Disconnect() = 0;
	virtual BOOL BroadcastEcho() = 0;
	virtual BOOL RemovePeer(BYTE byIndex) = 0;
	virtual BOOL DataSend( const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff) = 0;

protected:
	IUDPNetworkCallback* m_pUDPCallback;

private:
	
};