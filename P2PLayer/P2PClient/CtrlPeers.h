////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 
// 설  명 :
//

#ifndef __CTRLPEERS_H__
#define __CTRLPEERS_H__

////////////////////////////////////////////////////////////////////////////////
//
class CCtrlPeers: public CUdpCommClient
{
	typedef void (CCtrlPeers::*CTRLPEERSFUNC)(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);

public:
	CCtrlPeers();
	CCtrlPeers(BOOL bbRcvPktQueue);
	virtual ~CCtrlPeers();

public:
	virtual BOOL Init();
	virtual void Uninit();

public:
	virtual void HandleIdle();
	virtual void HandleError(OCFCLIENT_ERROR_CATEGORY enumErrorCategory, LONG ErrorCode, unsigned char chServiceType, ULONG ulLen, const BYTE* pbyBuff);

public:
	virtual void Pop(const SOCKADDR_IN& stRemote, ULONG ulLen, const BYTE* pbyBuff);
	virtual void OnReqEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnResEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;

	virtual void OnReqConnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnResConnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;

	//
	virtual void OnReqCheckRelayAblePeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnResCheckRelayAblePeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;

	virtual void OnReqRelayConnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnResRelayConnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnReqDisconnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnResDisconnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnReqRelayDisconnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnResRelayDisconnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnRelayDataPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnRelayDataPeerEx(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;

	virtual void OnNewPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void OnReqCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType) = 0;
	virtual void OnResCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulRemoteE, USHORT uiRemotePortE) = 0;
	virtual void OnQueueDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff) = 0;

public:
	virtual BOOL CheckReqConnect(ULONG ulLocalE, USHORT uiLocalPortE);

public:
	void SetRelayInfo(const char* pszRemoteR, USHORT uiRemotePortR);
	void SendCheckExternalIp();
	void GetSelfExternalInfo(SOCKADDR_IN& stRemoteE);
	void GetSelfExternalInfo(char achRemoteE[PU_MAXDOTADDRESS_LEN + 1], USHORT& uiRemotePortE);
	void GetSelfLocalInfo(SOCKADDR_IN& sinLocal);
	void GetSelfLocalInfo(char achLocal[PU_MAXDOTADDRESS_LEN + 1], USHORT& uiLocalPortE);
	BOOL AddPeer(const char* pszRemoteL, USHORT uiRemotePortL, const char* pszRemoteE, USHORT uiRemotePortE, const char* pszRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex);
	BOOL AddPeer(ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex);

	BOOL FindPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE);
	BOOL FindPeer(BYTE& byIndex, const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE);
	BOOL FindPeer(BYTE& byIndex, ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE);

	BOOL UpdatePeerE2(BYTE byIndex, ULONG ulRemoteE2, USHORT uiRemotePortE2);
	// 지우는게 peer릴레이면 내부에서 InitRelayAblePeer() 시켜 줌
	BOOL DelPeer(BYTE byIndex);
	BOOL IsPeerConnection(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE);
	BOOL IsPeerConnection(BYTE byIndex);
	void ResetPeers();
	BOOL PeerConnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType, const SOCKADDR_IN& stRemote);
	BOOL PeerConnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType);
	BOOL PeerDisconnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType, const SOCKADDR_IN& stRemote);
	BOOL PeerDisconnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType);
	BOOL TryConnectForPeers();
	BOOL TryConnectForLocalPeers();
	BOOL TryConnectForExternal2Peers();
	BOOL TryConnectForRelayPeers();
	BOOL TryDisonnectForPeers();
	BOOL TryNewPeerForPeers(ULONG ulAddrL, USHORT uiPortL, ULONG ulAddrE, USHORT uiPortE);
	void TryNewPeerForEachOther();
	BOOL TryEchoForPeers();
	BOOL TryDataForPeers(USHORT uiLen, const BYTE* pbyData);
	BOOL TryDataForPeer(PU_PEER_ADDRESS &stAdd, USHORT uiLen, const BYTE* pbyData);
	BOOL TryDataWithRelayForPeers(USHORT uiLen, const BYTE* pbyData);
	void TryCheckPeers(USHORT uiPeriod);
	BOOL TryCheckRelayAblePeer();
	BOOL GetPeersInfo(ULONG& ulCount, CCtrlMem& cInfo); 
	void Dispatch(BOOL bForce);
	void ClearInQueue();
	CPeerInfoNode GetIsConnect( ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE);

	BOOL TryDataSend( const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff);

	// 릴레이 가능한 피어인지 체크
	inline void SetRelayAblePeerFlag( BYTE byCheckRelayAblePeer ) { _byCheckRelayAblePeer = byCheckRelayAblePeer; }
	inline BYTE GetRelayAblePeer() { return _byCheckRelayAblePeer; }

	BOOL PeerCheckRelayAblePeer(BYTE byIndex);
	
	//
	BOOL AddPeerEx(const char* pszRemoteL, USHORT uiRemotePortL, const char* pszRemoteE, USHORT uiRemotePortE, const char* pszRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex, int iDatatype);
	BOOL AddPeerEx(ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex, int iDatatype);
// 	BOOL AddPeerEx(const char* pszRemoteL, USHORT uiRemotePortL, const char* pszRemoteE, USHORT uiRemotePortE, const char* pszRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex, int iDatatype, int iRelayLevel);
// 	BOOL AddPeerEx(ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex, int iDatatype, int iRelayLevel);

	BOOL TryDataForPeersEx(USHORT uiLen, const BYTE* pbyData, UINT iDataType);
	BOOL FindPeer(BYTE byIndex, CPeerInfoNode& pcSrchPeerInfo);
	BOOL ModifyPeer(CPeerInfoNode cSrchPeerInfo);
	BOOL ModifyMultiMediaPeers(UINT uiMask, BOOL bAndOR);

	// 1:1 전용
	BOOL SetSinglePeer( ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2,  int iDatatype ); // 전송할 피어를 설정합니다
	BOOL TryCheckRelayAbleSinglePeer();	// 대상 피어가 릴레이가 가능한지 확인하고 대상 피어의 포트를 다시 설정합니다
	BOOL TryDataForSinglePeer( USHORT uiLen, const BYTE* pbyData); //대상 피어에 데이터를 전송합니다
	BOOL PeerCheckRelayAbleSinglPeer( SOCKADDR_IN stRemote );

	//
	BOOL SendSpeedChk(const char* pszIP, USHORT uiPort);

public:
	inline void SetReceivePktQueue(BOOL bRcvPktQueue) { m_bRcvPktQueue = bRcvPktQueue; }
	inline BOOL GetReceivePktQueue() { return m_bRcvPktQueue; }

protected:
	void SetSelfExternalInfo(const char* pszRemoteE, USHORT uiRemotePortE);
	void SetSelfExternalInfo(ULONG ulRemoteE, USHORT uiRemotePortE);

private:
	BOOL SendRelayData(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PKT_HEAD_TYPE enumType, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL SendData(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL SendReqEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL SendResEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);

	// 접속 요청
	BOOL SendReqSyn(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	// 접속 요청에 대한 응답
	BOOL SendResSyn(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);

	BOOL SendReqCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq);
	BOOL SendResCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq);

	BOOL SendReqFin(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL SendResFin(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL SendReset(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL SendNewPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	BOOL Send(const SOCKADDR_IN& stRemote, BYTE byType, BYTE byAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);

	// 릴레이가능 요청
	BOOL SendReqCheckRelayAblePeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);
	// 릴레이가능 요청에 대한 응답
	BOOL SendResCheckRelayAblePeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);

	// 많은 사람 모아서 릴레이 하기
	BOOL SendRelayDataEx(const SOCKADDR_IN& stRemote, vector<SOCKADDR_IN>& vectorStDstRemote, PU_PKT_HEAD_TYPE enumType, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff);

private:
	BOOL UpdatePeerRcvTime(BYTE byIndex, PU_PEERADDRTYPE enumAddrType);
	BOOL UpdatePeerAllTime(BYTE byIndex, PU_PEERADDRTYPE enumAddrType);

private:
	void PktHandleReqEcho(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleResEcho(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);

	// 접속 요청
	void PktHandleReqSyn(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleResSyn(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);

	void PktHandleReqFin(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleResFin(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleReset(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);

	void PktHandleData(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);	// 피어
	void PktHandleRelayData(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);	// 릴레이
	void PktHandleRelayDataEx(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);	// 릴레이

	void PktHandleNewPeer(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleReqCheckExternalIp(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleResCheckExternalIp(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);

	// 릴레이가능
	void PktHandleReqCheckRelayAblePeer(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);
	void PktHandleResCheckRelayAblePeer(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff);

private:
	CTRLPEERSFUNC m_pfFunc[PU_PKT_HEAD_TYPE_END];

private:
	ULONG m_ulLb;
	USHORT _usDataIdx;
	BOOL m_bRcvPktQueue;
	BYTE _byCheckRelayAblePeer;

	SOCKADDR_IN m_stRelayPeer;
	SOCKADDR_IN m_stSelfExternal;
	SOCKADDR_IN m_stSelfLocal;

private:
	CMemPool<CUdpPacketNode> m_cRcvPktPool; 
	CUdpPacketList m_cRcvPktList;
	CPeerInfoList m_cPeerInfoList;

	//1:1전용 Peer;
	CPeerInfoNode m_cSingPeer;

private:
	CCriticalClient m_cRcvCtrlLock;
	CCriticalClient m_cPeerCtrlLock;

private:
	BOOL m_bRelayServerFirst;
};

#endif
