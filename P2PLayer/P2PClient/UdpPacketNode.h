////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.03.08
// 설  명 :
//

#ifndef __UDPPACKETNODE_H__
#define __UDPPACKETNODE_H__

////////////////////////////////////////////////////////////////////////////////
//
class CUdpPacketNode: public CNode
{
public:
	CUdpPacketNode();
	virtual ~CUdpPacketNode();
	
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual LONG CompareTo(CNode *pNode);

public:
	BOOL Init(const SOCKADDR_IN& stRemote, LONG lLen, const BYTE *pchBuff);
	BOOL Init(const SOCKADDR_IN& stRemote, LONG lLenHd, const BYTE *pchHdBuff, LONG lLenBd, const BYTE *pchBdBuff);
	void Uninit();

public:
	inline DWORD GetWrCurr() { return m_lWrCurr; }
	inline void SetWrCurr(LONG lWrCurr) { m_lWrCurr += lWrCurr; }
	inline DWORD GetLen() { return (m_lLen - m_lWrCurr); }
	inline BYTE* GetBuff() { return (m_pbyBuff + m_lWrCurr); }
	inline BYTE* GetRawBuff() { return m_pbyBuff; }
	inline ULONG GetRawLen() { return m_lLen; }
	inline SOCKADDR_IN& GetRemoteAddress() { return m_stRemote; }
	inline int GetRemoteAddressLength() { return sizeof(m_stRemote); }

private:
	BYTE* m_pbyBuff;
	LONG m_lWrCurr;
	LONG m_lLen;

private:
	SOCKADDR_IN m_stRemote;
	CMemChunk m_cChunk;

private:
	ULONG m_cRefs;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 
class CUdpPacketList: public CBoList
{
public:
	CUdpPacketList();

public:
	void Add(CUdpPacketNode* pcNode);
	void Delete(CUdpPacketNode* pcNode);
	LONG GetNodeCount();

private:
	LONG m_lNodeCount;
};

#endif
