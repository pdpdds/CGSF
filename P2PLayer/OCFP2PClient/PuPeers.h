//
//
#ifndef OCF_P2P_H
#define OCF_P2P_H

#include "AccessAll.h"
#include <map>
#include <vector>
#include <algorithm>
#include "Data.h"

using namespace std;

#pragma warning( push )
#pragma warning( disable:4100 )

//
class CPuPeers: public CCtrlPeers
{
public:
	static CPuPeers& GetInstance()
	{
		static CPuPeers Instance;
		return Instance;
	}

public:
	CPuPeers();
	virtual ~CPuPeers();

	virtual void OnTest(const SOCKADDR_IN& stRemote ) {}

	virtual void OnReqEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnResEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	void OnReqConnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) {}
	virtual void OnResConnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnReqRelayConnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnResRelayConnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	void OnReqDisconnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) {}
	virtual void OnResDisconnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnReqRelayDisconnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnResRelayDisconnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnRelayDataPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnNewPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);

	// 서버에 접속되면 외부 ip가 넘어 온다.
	virtual void OnReqCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType);
	virtual void OnResCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulRemoteE, USHORT uiRemotePortE);

	virtual void OnQueueDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff);
	virtual void HandleIdle();
	virtual void HandleError(OCFCLIENT_ERROR_CATEGORY enumErrorCategory, LONG ErrorCode, unsigned char chServiceType, ULONG ulLen, const BYTE* pbyBuff);
	virtual BOOL CheckReqConnect(ULONG ulLocalE, USHORT uiLocalPortE);

	virtual void ProcessDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) {}

	virtual void OnReqCheckRelayAblePeer(const SOCKADDR_IN &,PU_PEERADDRTYPE,ULONG,const BYTE *){}
	virtual void OnResCheckRelayAblePeer(const SOCKADDR_IN &,PU_PEERADDRTYPE,ULONG,const BYTE *){}

	virtual void OnRelayDataPeerEx(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) {}
};

#pragma warning( pop )

#endif