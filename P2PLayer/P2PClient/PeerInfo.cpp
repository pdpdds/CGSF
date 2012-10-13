////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2006.11.09
// 설  명 :
//

////////////////////////////////////////////////////////////////////////////////
//
#include "AccessAll.h"

////////////////////////////////////////////////////////////////////////////////
//
CPeerInfoNode::CPeerInfoNode(BYTE byIndex):
	m_byIndex(byIndex),
	m_cRefs(1)
{
	//
	srand(::GetTickCount());

	m_lPktSeq = rand();

	//
	m_enumPeerAddrType = PU_PEERADDRTYPE_NONE;
	m_enumCompCondition = PU_COMPCONDITION_INDEX;

	//
	m_bConnFlagL = FALSE;
	m_bConnFlagE = FALSE;
	m_bConnFlagE2 = FALSE;
	m_bConnFlagR = FALSE;
	_bRelayAblePeer = FALSE;

	//
	memset(&m_sinRemoteL, 0x00, sizeof(m_sinRemoteL));
	memset(&m_sinRemoteE, 0x00, sizeof(m_sinRemoteE));
	memset(&m_sinRemoteE2, 0x00, sizeof(m_sinRemoteE2));
	memset(&m_sinRemoteR, 0x00, sizeof(m_sinRemoteR));

	//
	m_tLastRcvTimeL = 0;
	m_tLastRcvTimeE = 0;
	m_tLastRcvTimeE2 = 0;
	m_tLastRcvTimeR = 0;

	//
	m_tLastSndTimeL = 0;
	m_tLastSndTimeE = 0;
	m_tLastSndTimeE2 = 0;
	m_tLastSndTimeR = 0;

	m_iDataType =0;
	m_iRelayLevel =0;
}

CPeerInfoNode::CPeerInfoNode():
	m_cRefs(1)
{
	m_byIndex = 0;

	//
	m_lPktSeq = 0;

	//
	m_enumPeerAddrType = PU_PEERADDRTYPE_NONE;
	m_enumCompCondition = PU_COMPCONDITION_INDEX;

	//
	m_bConnFlagL = FALSE;
	m_bConnFlagE = FALSE;
	m_bConnFlagE2 = FALSE;
	m_bConnFlagR = FALSE;

	//
	memset(&m_sinRemoteL, 0x00, sizeof(m_sinRemoteL));
	memset(&m_sinRemoteE, 0x00, sizeof(m_sinRemoteE));
	memset(&m_sinRemoteE2, 0x00, sizeof(m_sinRemoteE2));
	memset(&m_sinRemoteR, 0x00, sizeof(m_sinRemoteR));

	//
	m_tLastRcvTimeL = 0;
	m_tLastRcvTimeE = 0;
	m_tLastRcvTimeE2 = 0;
	m_tLastRcvTimeR = 0;

	//
	m_tLastSndTimeL = 0;
	m_tLastSndTimeE = 0;
	m_tLastSndTimeE2 = 0;
	m_tLastSndTimeR = 0;

	m_iDataType =0;
	m_iRelayLevel =0;
}

CPeerInfoNode::~CPeerInfoNode()
{
}

ULONG 
CPeerInfoNode::AddRef()
{
	return (++m_cRefs);
}

ULONG 
CPeerInfoNode::Release()
{
	if (0 == --m_cRefs)
	{
		delete this;
		return 0;
	}

	return m_cRefs;
}

LONG 
CPeerInfoNode::CompareTo(CNode *pNode)
{
	CPeerInfoNode* pCmp = static_cast<CPeerInfoNode*>(pNode);

	if (PU_COMPCONDITION_INDEX == pCmp->GetCompCondition())
	{
		return pCmp->GetIndex() - m_byIndex;
	}
	else
	if (PU_COMPCONDITION_ADDRESS_L == pCmp->GetCompCondition())
	{
		if (pCmp->GetRemoteL().sin_addr.S_un.S_addr == m_sinRemoteL.sin_addr.S_un.S_addr && pCmp->GetRemoteL().sin_port == m_sinRemoteL.sin_port)
		{
			return 0;
		}
	}
	else
	if (PU_COMPCONDITION_ADDRESS_E == pCmp->GetCompCondition())
	{
		if (pCmp->GetRemoteE().sin_addr.S_un.S_addr == m_sinRemoteE.sin_addr.S_un.S_addr && pCmp->GetRemoteE().sin_port == m_sinRemoteE.sin_port)
		{
			return 0;
		}
	}
	else
	if (PU_COMPCONDITION_ADDRESS_L_OR_E == pCmp->GetCompCondition())
	{
		if ((pCmp->GetRemoteL().sin_addr.S_un.S_addr == m_sinRemoteL.sin_addr.S_un.S_addr && pCmp->GetRemoteL().sin_port == m_sinRemoteL.sin_port) ||
			(pCmp->GetRemoteE().sin_addr.S_un.S_addr == m_sinRemoteE.sin_addr.S_un.S_addr && pCmp->GetRemoteE().sin_port == m_sinRemoteE.sin_port))
		{
			return 0;
		}
	}
	else
	if (PU_COMPCONDITION_ADDRESS_L_AND_E == pCmp->GetCompCondition())
	{
		if ((pCmp->GetRemoteL().sin_addr.S_un.S_addr == m_sinRemoteL.sin_addr.S_un.S_addr && pCmp->GetRemoteL().sin_port == m_sinRemoteL.sin_port) &&
			(pCmp->GetRemoteE().sin_addr.S_un.S_addr == m_sinRemoteE.sin_addr.S_un.S_addr && pCmp->GetRemoteE().sin_port == m_sinRemoteE.sin_port))
		{
			return 0;
		}
	}

	return 1;
}

void
CPeerInfoNode::SetRemoteL(const char* pszRemoteL, USHORT uiRemotePortL)
{
	m_sinRemoteL.sin_family = AF_INET;
	m_sinRemoteL.sin_addr.s_addr = inet_addr(pszRemoteL);
	m_sinRemoteL.sin_port = htons(uiRemotePortL);
}

void
CPeerInfoNode::SetRemoteE(const char* pszRemoteE, USHORT uiRemotePortE)
{
	m_sinRemoteE.sin_family = AF_INET;
	m_sinRemoteE.sin_addr.s_addr = inet_addr(pszRemoteE);
	m_sinRemoteE.sin_port = htons(uiRemotePortE);
}

void
CPeerInfoNode::SetRemoteE2(const char* pszRemoteE2, USHORT uiRemotePortE2)
{
	m_sinRemoteE2.sin_family = AF_INET;
	m_sinRemoteE2.sin_addr.s_addr = inet_addr(pszRemoteE2);
	m_sinRemoteE2.sin_port = htons(uiRemotePortE2);
}

void
CPeerInfoNode::SetRemoteR(const char* pszRemoteR, USHORT uiRemotePortR)
{
	m_sinRemoteR.sin_family = AF_INET;
	m_sinRemoteR.sin_addr.s_addr = inet_addr(pszRemoteR);
	m_sinRemoteR.sin_port = htons(uiRemotePortR);
}

void
CPeerInfoNode::SetRemoteL(ULONG ulRemoteL, USHORT uiRemotePortL)
{
	m_sinRemoteL.sin_family = AF_INET;
	m_sinRemoteL.sin_addr.s_addr = ulRemoteL;
	m_sinRemoteL.sin_port = uiRemotePortL;
}

void
CPeerInfoNode::SetRemoteE(ULONG ulRemoteE, USHORT uiRemotePortE)
{
	m_sinRemoteE.sin_family = AF_INET;
	m_sinRemoteE.sin_addr.s_addr = ulRemoteE;
	m_sinRemoteE.sin_port = uiRemotePortE;
}

void
CPeerInfoNode::SetRemoteE2(ULONG ulRemoteE2, USHORT uiRemotePortE2)
{
	m_sinRemoteE2.sin_family = AF_INET;
	m_sinRemoteE2.sin_addr.s_addr = ulRemoteE2;
	m_sinRemoteE2.sin_port = uiRemotePortE2;
}

void
CPeerInfoNode::SetRemoteR(ULONG ulRemoteR, USHORT uiRemotePortR)
{
	m_sinRemoteR.sin_family = AF_INET;
	m_sinRemoteR.sin_addr.s_addr = ulRemoteR;
	m_sinRemoteR.sin_port = uiRemotePortR;
}

ULONG
CPeerInfoNode::GetIncSeq()
{
	return InterlockedIncrement(&m_lPktSeq);
}

void
CPeerInfoNode::SetDataType(int iDataType)
{
	m_iDataType = iDataType;
}

void
CPeerInfoNode::SetRelayLevel(int iRelayLevel)
{
	m_iRelayLevel = iRelayLevel;
}


////////////////////////////////////////////////////////////////////////////////
//
CPeerInfoList::CPeerInfoList()
{
	CPeerInfoNode* pcNewPeerInfo;

	//
	memset(m_apcPeers, 0x00, sizeof(m_apcPeers));

	//
	for (BYTE byStep = 1; byStep < PU_PEERS_MAXCOUNT; byStep++)
	{
		pcNewPeerInfo = new CPeerInfoNode(byStep);
		if (NULL != pcNewPeerInfo)
		{
			m_cPeerInfoPool.Deallocate(pcNewPeerInfo);
		}
	}
}

BOOL
CPeerInfoList::Add(CPeerInfoNode* pcNode)
{
	if (NULL != m_apcPeers[pcNode->GetIndex()])
	{
		return FALSE;
	}

	//
	m_apcPeers[pcNode->GetIndex()] = pcNode;

	//
	CBoList::Add(pcNode);

	return TRUE;
}

BOOL
CPeerInfoList::Delete(BYTE byIndex)
{
	CPeerInfoNode* pcDelPeerInfo;

	//
	if (NULL == m_apcPeers[byIndex])
	{
		return FALSE;
	}

	//
	pcDelPeerInfo = static_cast<CPeerInfoNode*>(CBoList::DelWithLiveInPtr(m_apcPeers[byIndex]));
	if (NULL == pcDelPeerInfo)
	{
		return FALSE;
	}

	//
	m_cPeerInfoPool.Deallocate(pcDelPeerInfo);

	//
	m_apcPeers[byIndex] = NULL; 

	return TRUE;
}

void
CPeerInfoList::Reset()
{
	CPeerInfoNode* pcDelPeerInfo;

	while (FALSE == CBoList::IsEmpty())
	{
		pcDelPeerInfo = static_cast<CPeerInfoNode*>(CBoList::DelBeginWithLive());

		m_apcPeers[pcDelPeerInfo->GetIndex()] = NULL; 
		m_cPeerInfoPool.Deallocate(pcDelPeerInfo);
	}
}

CPeerInfoNode*
CPeerInfoList::Allocate()
{
	if (0 == m_cPeerInfoPool.GetCount())
	{
		return NULL;
	}

	return m_cPeerInfoPool.Allocate();
}

void
CPeerInfoList::Deallocate(CPeerInfoNode* pcPeerInfoNode)
{
	m_cPeerInfoPool.Deallocate(pcPeerInfoNode);
}
