////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2002.03.18
// 설  명 :
// 수정일 : 2007.11.13
//

////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfClientSys.h"
#include "AllOcfClient.h"

////////////////////////////////////////////////////////////////////////////////
//
CUdpPacketNode::CUdpPacketNode():
	m_cRefs(1)
{
	m_lWrCurr = 0;
	m_lLen = 0;
	m_pbyBuff = NULL;
}

CUdpPacketNode::~CUdpPacketNode()
{
}

void	
CUdpPacketNode::Uninit()
{
	m_lWrCurr = 0;
	m_lLen = 0;
}

ULONG 
CUdpPacketNode::AddRef()
{
	return (++m_cRefs);
}

ULONG 
CUdpPacketNode::Release()
{
	if (0 == --m_cRefs)
	{
		delete this;
		return 0;
	}

	return m_cRefs;
}

BOOL 
CUdpPacketNode::Init(const SOCKADDR_IN& stRemote, LONG lLen, const BYTE *pbyBuff)
{
	if (FALSE == m_cChunk.Ext(lLen + 1))
	{
		return FALSE;
	}

	//
	m_lLen = lLen;
	m_pbyBuff = reinterpret_cast<BYTE*>(m_cChunk.GetMemChunk());

	//
	memcpy(&m_stRemote, &stRemote, sizeof(m_stRemote));

	//
	memcpy(m_pbyBuff, pbyBuff, lLen);

	return TRUE;
}

BOOL
CUdpPacketNode::Init(const SOCKADDR_IN& stRemote, LONG lLenHd, const BYTE *pchHdBuff, LONG lLenBd, const BYTE *pchBdBuff)
{
	if (FALSE == m_cChunk.Ext(lLenHd + lLenBd + 1))
	{
		return FALSE;
	}

	//
	m_lLen = lLenHd + lLenBd;
	m_pbyBuff = reinterpret_cast<BYTE*>(m_cChunk.GetMemChunk());

	//
	memcpy(&m_stRemote, &stRemote, sizeof(m_stRemote));

	//
	memcpy(m_pbyBuff, pchHdBuff, lLenHd);
	memcpy(m_pbyBuff + lLenHd, pchBdBuff, lLenBd);

	return TRUE;
}

LONG 
CUdpPacketNode::CompareTo(CNode *pNode)
{
	CUdpPacketNode* pCmp = static_cast<CUdpPacketNode*>(pNode);
	return (LONG(pCmp) - LONG(this));
}

/////////////////////////////////////////////////////////////////////////////////////////////
// 
CUdpPacketList::CUdpPacketList(): 
	m_lNodeCount(0)
{
}

void 
CUdpPacketList::Add(CUdpPacketNode* pcNode)
{
	m_lNodeCount++;
	CBoList::Add(pcNode);
}

void 
CUdpPacketList::Delete(CUdpPacketNode* pcNode)
{
	m_lNodeCount--;
	CBoList::Delete(pcNode);
}

LONG 
CUdpPacketList::GetNodeCount()
{
	return m_lNodeCount;
}
