////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.11.16
// 설  명 :
//

#ifndef __PEERINFO_H__
#define __PEERINFO_H__

////////////////////////////////////////////////////////////////////////////////
//
#define PU_PEERS_MAXCOUNT						255

#pragma warning( push )
#pragma warning( disable:4244 )


////////////////////////////////////////////////////////////////////////////////
//
// 통신 타입을 정하기 위해서...
//
typedef enum
{
	PU_PEERADDRTYPE_NONE = 0,
	PU_PEERADDRTYPE_LOCAL = 1,
	PU_PEERADDRTYPE_EXTERNAL = 2,
	PU_PEERADDRTYPE_EXTERNAL2 = 3,
	PU_PEERADDRTYPE_RELAY = 4,
	PU_PEERADDRTYPE_RELAY_PEER = 5
} PU_PEERADDRTYPE;

//
typedef enum
{
	PU_COMPCONDITION_INDEX = 1,
	PU_COMPCONDITION_ADDRESS_L = 2,
	PU_COMPCONDITION_ADDRESS_E = 3,
	PU_COMPCONDITION_ADDRESS_L_OR_E = 4,
	PU_COMPCONDITION_ADDRESS_L_AND_E = 5
} PU_COMPCONDITION;

//
typedef struct
{
	BYTE m_byIndex;

	BOOL m_bConnectL;
	BOOL m_bConnectE;
	BOOL m_bConnectE2;
	BOOL m_bConnectR;
	BOOL _bRelayAblePeer;

	time_t m_tLastRcvTimeL;
	time_t m_tLastRcvTimeE;
	time_t m_tLastRcvTimeE2;
	time_t m_tLastRcvTimeR;

	time_t m_tLastSndTimeL;
	time_t m_tLastSndTimeE;
	time_t m_tLastSndTimeE2;
	time_t m_tLastSndTimeR;

	SOCKADDR_IN m_stRemoteL;
	SOCKADDR_IN m_stRemoteE;
	SOCKADDR_IN m_stRemoteE2;
	SOCKADDR_IN m_stRemoteR;

	int m_iDataType;
} PU_PEERINFO;


typedef struct
{
	BOOL m_bUserble;
//	BYTE m_byIndex;

	BOOL m_bConnectL;
	BOOL m_bConnectE;
	BOOL m_bConnectE2;
	BOOL m_bConnectR;
//	BOOL _bRelayAblePeer;

	time_t m_tLastRcvTimeL;
	time_t m_tLastRcvTimeE;
	time_t m_tLastRcvTimeE2;
	time_t m_tLastRcvTimeR;

	time_t m_tLastSndTimeL;
	time_t m_tLastSndTimeE;
	time_t m_tLastSndTimeE2;
	time_t m_tLastSndTimeR;

	SOCKADDR_IN m_stRemoteL;
	SOCKADDR_IN m_stRemoteE;
	SOCKADDR_IN m_stRemoteE2;
	SOCKADDR_IN m_stRemoteR;

} PU_PEERINFO_ONE;

////////////////////////////////////////////////////////////////////////////////
//
class CPeerInfoNode: public CNode
{
public:
	CPeerInfoNode(BYTE byIndex);
	CPeerInfoNode();
	virtual ~CPeerInfoNode();
	
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual LONG CompareTo(CNode *pNode);

public:
	inline BYTE GetIndex() { return m_byIndex; }

	inline void SetPeerAddrType(PU_PEERADDRTYPE enumPeerAddrType) { m_enumPeerAddrType = enumPeerAddrType; }
	inline PU_PEERADDRTYPE GetPeerAddrType() { return m_enumPeerAddrType; }

	inline void SetCompCondition(PU_COMPCONDITION enumCompCondition) { m_enumCompCondition = enumCompCondition; }
	inline PU_COMPCONDITION GetCompCondition() { return m_enumCompCondition; }

	inline void SetConnFlagL(BOOL bConnFlagL) { m_bConnFlagL = bConnFlagL; }
	inline void SetConnFlagE(BOOL bConnFlagE) { m_bConnFlagE = bConnFlagE; }
	inline void SetConnFlagE2(BOOL bConnFlagE) { m_bConnFlagE2 = bConnFlagE; }
	inline void SetConnFlagR(BOOL bConnFlagR) { m_bConnFlagR = bConnFlagR; }
	inline void SetRelayAblePeer(BOOL bRelayAblePeer) { _bRelayAblePeer = bRelayAblePeer; }

	inline BOOL GetConnFlagL() { return m_bConnFlagL; }
	inline BOOL GetConnFlagE() { return m_bConnFlagE; }
	inline BOOL GetConnFlagE2() { return m_bConnFlagE2; }
	inline BOOL GetConnFlagR() { return m_bConnFlagR; }
	inline BOOL GetRelayAblePeer() { return _bRelayAblePeer; }

	inline void SetIndex(BOOL byIndex) { m_byIndex = byIndex;}

	inline void SetRemoteL(const SOCKADDR_IN& sinRemoteL) { memcpy(&m_sinRemoteL, &sinRemoteL, sizeof(m_sinRemoteL)); }
	inline void SetRemoteE(const SOCKADDR_IN& sinRemoteE) { memcpy(&m_sinRemoteE, &sinRemoteE, sizeof(m_sinRemoteE)); }
	inline void SetRemoteE2(const SOCKADDR_IN& sinRemoteE2) { memcpy(&m_sinRemoteE2, &sinRemoteE2, sizeof(m_sinRemoteE2)); }
	inline void SetRemoteR(const SOCKADDR_IN& sinRemoteR) { memcpy(&m_sinRemoteR, &sinRemoteR, sizeof(m_sinRemoteR)); }

	inline SOCKADDR_IN& GetRemoteL() { return m_sinRemoteL; }
	inline SOCKADDR_IN& GetRemoteE() { return m_sinRemoteE; }
	inline SOCKADDR_IN& GetRemoteE2() { return m_sinRemoteE2; }
	inline SOCKADDR_IN& GetRemoteR() { return m_sinRemoteR; }

	inline void SetLastRcvTimeL(time_t tLastRcvTime) { m_tLastRcvTimeL = tLastRcvTime; }
	inline void SetLastRcvTimeE(time_t tLastRcvTime) { m_tLastRcvTimeE = tLastRcvTime; }
	inline void SetLastRcvTimeE2(time_t tLastRcvTime) { m_tLastRcvTimeE2 = tLastRcvTime; }
	inline void SetLastRcvTimeR(time_t tLastRcvTime) { m_tLastRcvTimeR = tLastRcvTime; }

	inline void SetLastSndTimeL(time_t tLastSndTime) { m_tLastSndTimeL = tLastSndTime; }
	inline void SetLastSndTimeE(time_t tLastSndTime) { m_tLastSndTimeE = tLastSndTime; }
	inline void SetLastSndTimeE2(time_t tLastSndTime) { m_tLastSndTimeE2 = tLastSndTime; }
	inline void SetLastSndTimeR(time_t tLastSndTime) { m_tLastSndTimeR = tLastSndTime; }

	inline time_t GetLastRcvTimeL() { return m_tLastRcvTimeL; }
	inline time_t GetLastRcvTimeE() { return m_tLastRcvTimeE; }
	inline time_t GetLastRcvTimeE2() { return m_tLastRcvTimeE2; }
	inline time_t GetLastRcvTimeR() { return m_tLastRcvTimeR; }

	inline time_t GetLastSndTimeL() { return m_tLastSndTimeL; }
	inline time_t GetLastSndTimeE() { return m_tLastSndTimeE; }
	inline time_t GetLastSndTimeE2() { return m_tLastSndTimeE2; }
	inline time_t GetLastSndTimeR() { return m_tLastSndTimeR; }

	inline UINT GetDataType() { return m_iDataType; }
	inline UINT GetRelayLevel() { return m_iRelayLevel; }

public:
	void SetRemoteL(const char* pszRemoteL, USHORT uiRemotePortL);
	void SetRemoteE(const char* pszRemoteE, USHORT uiRemotePortE);
	void SetRemoteE2(const char* pszRemoteE2, USHORT uiRemotePortE2);
	void SetRemoteR(const char* pszRemoteR, USHORT uiRemotePortR);
	void SetRemoteL(ULONG ulRemoteL, USHORT uiRemotePortL);
	void SetRemoteE(ULONG ulRemoteE, USHORT uiRemotePortE);
	void SetRemoteE2(ULONG ulRemoteE2, USHORT uiRemotePortE2);
	void SetRemoteR(ULONG ulRemoteR, USHORT uiRemotePortR);
	ULONG GetIncSeq();

	void SetDataType(int iDataType);
	void SetRelayLevel(int iRelayLevel);


private:
	BYTE m_byIndex;

	PU_PEERADDRTYPE m_enumPeerAddrType;
	PU_COMPCONDITION m_enumCompCondition;
	LONG m_lPktSeq;

	BOOL m_bConnFlagL;
	BOOL m_bConnFlagE;
	BOOL m_bConnFlagE2;
	BOOL m_bConnFlagR;
	BOOL _bRelayAblePeer;

	SOCKADDR_IN m_sinRemoteL;				// 외부에서 보는 나의 로컬주소
	SOCKADDR_IN m_sinRemoteE;				// 외부에서 보는 나의 외부주소
	SOCKADDR_IN m_sinRemoteE2;				// 외부에서 보는 나의 외부주소2 - 릴레이서버에서 확인 해 줌
	SOCKADDR_IN m_sinRemoteR;				// 릴레이 서버

	time_t m_tLastRcvTimeL;
	time_t m_tLastRcvTimeE;
	time_t m_tLastRcvTimeE2;
	time_t m_tLastRcvTimeR;

	time_t m_tLastSndTimeL;
	time_t m_tLastSndTimeE;
	time_t m_tLastSndTimeE2;
	time_t m_tLastSndTimeR;

	int m_iDataType;	//데이터의 종류
	int m_iRelayLevel;

private:
	ULONG m_cRefs;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 
class CPeerInfoList: protected CBoList
{
public:
	CPeerInfoList();

public:
	BOOL Add(CPeerInfoNode* pcNode);
	BOOL Delete(BYTE byIndex);
	void Reset();

	CPeerInfoNode* Allocate();
	void Deallocate(CPeerInfoNode* pcPeerInfoNode);

public:
	inline BOOL IsEmpty() { return CBoList::IsEmpty(); }
	inline CPeerInfoNode* GetBegin() { return static_cast<CPeerInfoNode*>(CBoList::GetBegin()); }
	inline CPeerInfoNode* Find(CPeerInfoNode* pcNode) { return static_cast<CPeerInfoNode*>(CBoList::Find(pcNode)); }
	CPeerInfoNode* Find(BYTE byIndex) { return m_apcPeers[byIndex]; }


private:
	CMemPool<CPeerInfoNode> m_cPeerInfoPool; 

private:
	CPeerInfoNode* m_apcPeers[PU_PEERS_MAXCOUNT + 1];
};


#pragma warning( pop )
#endif
