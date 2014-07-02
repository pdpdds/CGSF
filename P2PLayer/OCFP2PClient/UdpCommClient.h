////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 
// 설  명 :
//

#ifndef __UDPCOMMCLIENT_H__
#define __UDPCOMMCLIENT_H__

#pragma warning( push )
#pragma warning( disable:4100 )

////////////////////////////////////////////////////////////////////////////////
//
class CUdpCommClient: public CCriticalClient, public CThreadClient
{
public:
	CUdpCommClient();
	virtual ~CUdpCommClient();

	virtual BOOL Init();
	virtual void Uninit();
	virtual BOOL DoEnd();
	virtual BOOL Do();
	virtual BOOL Push(const SOCKADDR_IN& stRemote, ULONG ulLen, const BYTE* pbyBuff);
	virtual BOOL Push(const SOCKADDR_IN& stRemote, ULONG ulLenHd, const BYTE* pbyHdBuff, ULONG ulLenBd, const BYTE* pbyBdBuff);
	virtual BOOL Push(CUdpPacketNode* pcPktNode);
	virtual void Pop(const SOCKADDR_IN& stRemote, ULONG ulLen, const BYTE* pbyBuff) = 0;
	virtual void Reset();
	virtual BOOL HandleRead();
	virtual BOOL HandleWrite();
	virtual void HandleIdle();
	virtual void HandleError(OCFCLIENT_ERROR_CATEGORY enumErrorCategory, LONG ErrorCode, unsigned char chServiceType = 0xFF, ULONG ulLen = 0, const BYTE* pbyBuff = NULL) {}

	// 디스패치방식 추가
//	virtual BOOL Start();
//	virtual BOOL End();
public:
	BOOL SetBindInfo(const char* pszAddr, UINT uiPort, ULONG ulMaxSndMsgPkLen, ULONG ulMaxRcvMsgPkLen, ULONG ulSndBuff, ULONG ulRcvBuff);
	// 쓰레드방식삭제
	void IdleWrite();

protected:
	BOOL GetLocalAddrInfo(SOCKADDR_IN& sinLocal);

public:
	// 쓰레드방식삭제
	inline void SetIdleTime(DWORD dwIdleTime) { m_dwIdleTime = dwIdleTime; }
	inline void SetSessionTimeout(LONG lSessionTimeout) { m_tSessionTimeout = lSessionTimeout; }
	inline int GetLastError() { return m_nErrorCode; }
	inline SOCKET GetSocketHandle() { return m_dSock; }
	inline BOOL GetConnectFlag() { return m_bConnectFlag; }

private:
	BOOL UdpFixBind();
	void OnWinSockEvent();

private:
	time_t m_tTimeout;
	time_t m_tSessionTimeout;
	// 쓰레드방식삭제
	DWORD m_dwIdleTime;
	BOOL m_bSockLibFlag;
	BOOL m_bConnectFlag;
	BOOL m_bWouldBlock;
	int m_nErrorCode;

private:
	CMemPool<CUdpPacketNode> m_cSndPktPool; 
	CUdpPacketList m_cSndPktList;
	CMemChunk m_cRecvChunk;

private:
	SOCKET m_dSock;
	SOCKADDR_IN m_sinLocal;
	ULONG m_ulMaxSndPktLen;
	ULONG m_ulMaxRcvPktLen;
	ULONG m_ulSndBuff;
	ULONG m_ulRcvBuff;

	ULONG _ulTick1;
	ULONG _ulTick2;

private:
	WSAEVENT m_hSockEvent;
	// 쓰레드방식삭제
	HANDLE m_hWrEvent;
	HANDLE m_hQuit;
};


#pragma warning( pop )
#endif
