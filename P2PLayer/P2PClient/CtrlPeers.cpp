////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 :
// 요  청 : 1. PeerInfo에서 검색할 때.... 비교 조건이 UNIQUE하도록 수정할 필요 있음..
//

////////////////////////////////////////////////////////////////////////////////
//
#include "AccessAll.h"

////////////////////////////////////////////////////////////////////////////////
//
CCtrlPeers::CCtrlPeers()
{
	//
	m_bRelayServerFirst = TRUE;

	//
	_usDataIdx = 0;
	m_ulLb = 0;

	//
	m_bRcvPktQueue = TRUE;

	//
	SetRelayAblePeerFlag( FALSE );

	//
	m_stRelayPeer.sin_family = AF_INET;
	m_stRelayPeer.sin_addr.s_addr = ADDR_ANY;
	m_stRelayPeer.sin_port = 0;

	//
	m_stSelfExternal.sin_family = AF_INET;
	m_stSelfExternal.sin_addr.s_addr = ADDR_ANY;
	m_stSelfExternal.sin_port = 0;

	//
	m_stSelfLocal.sin_family = AF_INET;
	m_stSelfLocal.sin_addr.s_addr = ADDR_ANY;
	m_stSelfLocal.sin_port = 0;

	//
	memset(m_pfFunc, 0x00, sizeof(m_pfFunc));

	//
	m_pfFunc[PU_PKT_HEAD_TYPE_REQECHO] = &CCtrlPeers::PktHandleReqEcho;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESECHO] = &CCtrlPeers::PktHandleResEcho;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQSYN] = &CCtrlPeers::PktHandleReqSyn;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESSYN] = &CCtrlPeers::PktHandleResSyn;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQFIN] = &CCtrlPeers::PktHandleReqFin;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESFIN] = &CCtrlPeers::PktHandleResFin;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESET] = &CCtrlPeers::PktHandleReset;
	m_pfFunc[PU_PKT_HEAD_TYPE_DATA] = &CCtrlPeers::PktHandleData;
	m_pfFunc[PU_PKT_HEAD_TYPE_RELAYDATA] = &CCtrlPeers::PktHandleRelayData;
	m_pfFunc[PU_PKT_HEAD_TYPE_NEWPEER] = &CCtrlPeers::PktHandleNewPeer;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQCHECKEXTERNALIP] = &CCtrlPeers::PktHandleReqCheckExternalIp;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESCHECKEXTERNALIP] = &CCtrlPeers::PktHandleResCheckExternalIp;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQCHECKRELAYABLEPEER] = &CCtrlPeers::PktHandleReqCheckRelayAblePeer;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESCHECKRELAYABLEPEER] = &CCtrlPeers::PktHandleResCheckRelayAblePeer;

	m_pfFunc[PU_PKT_HEAD_TYPE_RELAYDATA_EX] = &CCtrlPeers::PktHandleRelayDataEx;
//	m_pfFunc[PU_PKT_HEAD_TYPE_SENDSPEED] = &CCtrlPeers::PktHandleSendSpeed;

}

////////////////////////////////////////////////////////////////////////////////
//
CCtrlPeers::CCtrlPeers(BOOL bbRcvPktQueue)
{
	//

	m_bRcvPktQueue = bbRcvPktQueue;

	//
	m_stRelayPeer.sin_family = AF_INET;
	m_stRelayPeer.sin_addr.s_addr = ADDR_ANY;
	m_stRelayPeer.sin_port = 0;

	//
	m_stSelfExternal.sin_family = AF_INET;
	m_stSelfExternal.sin_addr.s_addr = ADDR_ANY;
	m_stSelfExternal.sin_port = 0;

	//
	m_stSelfLocal.sin_family = AF_INET;
	m_stSelfLocal.sin_addr.s_addr = ADDR_ANY;
	m_stSelfLocal.sin_port = 0;

	//
	memset(m_pfFunc, 0x00, sizeof(m_pfFunc));

	//
	m_pfFunc[PU_PKT_HEAD_TYPE_REQECHO] = &CCtrlPeers::PktHandleReqEcho;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESECHO] = &CCtrlPeers::PktHandleResEcho;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQSYN] = &CCtrlPeers::PktHandleReqSyn;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESSYN] = &CCtrlPeers::PktHandleResSyn;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQFIN] = &CCtrlPeers::PktHandleReqFin;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESFIN] = &CCtrlPeers::PktHandleResFin;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESET] = &CCtrlPeers::PktHandleReset;
	m_pfFunc[PU_PKT_HEAD_TYPE_DATA] = &CCtrlPeers::PktHandleData;
	m_pfFunc[PU_PKT_HEAD_TYPE_RELAYDATA] = &CCtrlPeers::PktHandleRelayData;
	m_pfFunc[PU_PKT_HEAD_TYPE_NEWPEER] = &CCtrlPeers::PktHandleNewPeer;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQCHECKEXTERNALIP] = &CCtrlPeers::PktHandleReqCheckExternalIp;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESCHECKEXTERNALIP] = &CCtrlPeers::PktHandleResCheckExternalIp;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQCHECKRELAYABLEPEER] = &CCtrlPeers::PktHandleReqCheckRelayAblePeer;
	m_pfFunc[PU_PKT_HEAD_TYPE_RESCHECKRELAYABLEPEER] = &CCtrlPeers::PktHandleResCheckRelayAblePeer;
}

CCtrlPeers::~CCtrlPeers()
{
}

BOOL
CCtrlPeers::Init()
{
	if (FALSE == CUdpCommClient::Init())
	{
		return FALSE;
	}

	//
	if (FALSE == m_cRcvCtrlLock.IsActive())
	{
		return FALSE;
	}

	//
	if (FALSE == m_cPeerCtrlLock.IsActive())
	{
		return FALSE;
	}

	//
	CUdpCommClient::GetLocalAddrInfo(m_stSelfLocal);

	return TRUE;
}

void
CCtrlPeers::Uninit()
{
	m_stRelayPeer.sin_family = AF_INET;
	m_stRelayPeer.sin_addr.s_addr = ADDR_ANY;
	m_stRelayPeer.sin_port = 0;

	//
	m_stSelfExternal.sin_family = AF_INET;
	m_stSelfExternal.sin_addr.s_addr = ADDR_ANY;
	m_stSelfExternal.sin_port = 0;

	//
	CUdpCommClient::Uninit();
}

void
CCtrlPeers::Pop(const SOCKADDR_IN& stRemote, ULONG ulLen, const BYTE* pbyBuff)
{
	if (ulLen < PU_PACKET_HEAD_LEN)
	{
		return;
	}

	//
	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);

	//
	if (NULL != m_pfFunc[pstPuHd->m_byType])
	{
		(this->*m_pfFunc[pstPuHd->m_byType])(stRemote, pstPuHd, ulLen, pbyBuff);
	}
}

void
CCtrlPeers::HandleIdle()
{
//	ULONG ulLb = m_ulLb % 3;

	// 수시로 릴레이 서버에 IP확인 요청을 한다...
	if (ADDR_ANY != m_stRelayPeer.sin_addr.S_un.S_addr)
	{
		SendReqCheckExternalIp(m_stRelayPeer, PU_PEERADDRTYPE_EXTERNAL2, ::GetTickCount());
		TryConnectForPeers();
	}

	//
// 	if (0 == ulLb)
// 	{
// 		TryConnectForLocalPeers();
// 	}
// 	else
// 	if (1 == ulLb)
// 	{
// 		TryConnectForExternal2Peers();
// 	}
// 	else
// 	if (2 == ulLb)
// 	{
// 		TryConnectForRelayPeers();
// 	}
// 
// 	m_ulLb++;
}

void
CCtrlPeers::SetRelayInfo(const char* pszRemoteR, USHORT uiRemotePortR)
{
	m_stRelayPeer.sin_family = AF_INET;
	m_stRelayPeer.sin_addr.s_addr = inet_addr(pszRemoteR);
	m_stRelayPeer.sin_port = htons(uiRemotePortR);
}

void
CCtrlPeers::SendCheckExternalIp()
{
	// 수시로 릴레이 서버에 IP확인 요청을 한다...
	if (ADDR_ANY != m_stRelayPeer.sin_addr.S_un.S_addr)
	{
		SendReqCheckExternalIp(m_stRelayPeer, PU_PEERADDRTYPE_EXTERNAL2, ::GetTickCount());
	}
}

void
CCtrlPeers::SetSelfExternalInfo(const char* pszRemoteE, USHORT uiRemotePortE)
{
	m_stSelfExternal.sin_family = AF_INET;
	m_stSelfExternal.sin_addr.s_addr = inet_addr(pszRemoteE);
	m_stSelfExternal.sin_port = htons(uiRemotePortE);
}

void
CCtrlPeers::SetSelfExternalInfo(ULONG ulRemoteE, USHORT uiRemotePortE)
{
	m_stSelfExternal.sin_family = AF_INET;
	m_stSelfExternal.sin_addr.s_addr = ulRemoteE;
	m_stSelfExternal.sin_port = uiRemotePortE;
}

void
CCtrlPeers::GetSelfExternalInfo(SOCKADDR_IN& stRemoteE)
{
	stRemoteE.sin_family = m_stSelfExternal.sin_family;
	stRemoteE.sin_addr.s_addr = m_stSelfExternal.sin_addr.s_addr;
	stRemoteE.sin_port = m_stSelfExternal.sin_port;
}

void
CCtrlPeers::GetSelfExternalInfo(char achRemoteE[PU_MAXDOTADDRESS_LEN + 1], USHORT& uiRemotePortE)
{
	strncpy(achRemoteE, inet_ntoa(m_stSelfExternal.sin_addr), PU_MAXDOTADDRESS_LEN);
	achRemoteE[PU_MAXDOTADDRESS_LEN] = '\0';

	uiRemotePortE = ntohs(m_stSelfExternal.sin_port);
}

void
CCtrlPeers::GetSelfLocalInfo(SOCKADDR_IN& sinLocal)
{
	sinLocal.sin_family = m_stSelfLocal.sin_family;
	sinLocal.sin_addr.s_addr = m_stSelfLocal.sin_addr.s_addr;
	sinLocal.sin_port = m_stSelfLocal.sin_port;
}

void
CCtrlPeers::GetSelfLocalInfo(char achLocal[PU_MAXDOTADDRESS_LEN + 1], USHORT& uiLocalPortE)
{
	strncpy(achLocal, inet_ntoa(m_stSelfLocal.sin_addr), PU_MAXDOTADDRESS_LEN);
	achLocal[PU_MAXDOTADDRESS_LEN] = '\0';

	uiLocalPortE = ntohs(m_stSelfLocal.sin_port);
}

BOOL
CCtrlPeers::AddPeer(const char* pszRemoteL, USHORT uiRemotePortL, const char* pszRemoteE, USHORT uiRemotePortE, const char* pszRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex)
{
	CPeerInfoNode* pcPeerInfo = m_cPeerInfoList.Allocate();
	if (NULL == pcPeerInfo)
	{
		return FALSE;
	}
	else
	{
		byIndex = pcPeerInfo->GetIndex();
	}

	//
	time_t tTime = time(NULL);

	//
	pcPeerInfo->SetConnFlagL(FALSE);
	pcPeerInfo->SetConnFlagE(FALSE);
	pcPeerInfo->SetConnFlagE2(FALSE);
 	pcPeerInfo->SetConnFlagR(TRUE);
 	pcPeerInfo->SetRelayAblePeer(FALSE);

	pcPeerInfo->SetLastRcvTimeL(tTime);
	pcPeerInfo->SetLastRcvTimeE(tTime);
	pcPeerInfo->SetLastRcvTimeE2(tTime);
	pcPeerInfo->SetLastRcvTimeR(tTime);

	pcPeerInfo->SetLastSndTimeL(tTime);
	pcPeerInfo->SetLastSndTimeE(tTime);
	pcPeerInfo->SetLastSndTimeE2(tTime);
	pcPeerInfo->SetLastSndTimeR(tTime);

	pcPeerInfo->SetRemoteL(pszRemoteL, uiRemotePortL);
	pcPeerInfo->SetRemoteE(pszRemoteE, uiRemotePortE);
	pcPeerInfo->SetRemoteE2(pszRemoteE2, uiRemotePortE2);
	pcPeerInfo->SetRemoteR(m_stRelayPeer);

	//
	pcPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);

	//
	pcPeerInfo->SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);

	//
	m_cPeerCtrlLock.Lock();

	//
	if (NULL == m_cPeerInfoList.Find(pcPeerInfo))
	{
		m_cPeerInfoList.Add(pcPeerInfo);

		//
		m_cPeerCtrlLock.Unlock();

		return TRUE;
	}
	//

	//
	m_cPeerCtrlLock.Unlock();

	//
	m_cPeerInfoList.Deallocate(pcPeerInfo);

	//
	TryCheckRelayAblePeer();

	return FALSE;
}

BOOL
CCtrlPeers::AddPeer(ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex)
{
	CPeerInfoNode* pcPeerInfo = m_cPeerInfoList.Allocate();
	if (NULL == pcPeerInfo)
	{
		return FALSE;
	}
	else
	{
		byIndex = pcPeerInfo->GetIndex();
	}

	//
	time_t tTime = time(NULL);

	//
	pcPeerInfo->SetConnFlagL(FALSE);
	pcPeerInfo->SetConnFlagE(FALSE);
	pcPeerInfo->SetConnFlagE2(FALSE);
	pcPeerInfo->SetConnFlagR(TRUE);
 	pcPeerInfo->SetRelayAblePeer(FALSE);

	pcPeerInfo->SetLastRcvTimeL(tTime);
	pcPeerInfo->SetLastRcvTimeE(tTime);
	pcPeerInfo->SetLastRcvTimeE2(tTime);
	pcPeerInfo->SetLastRcvTimeR(tTime);

	pcPeerInfo->SetLastSndTimeL(tTime);
	pcPeerInfo->SetLastSndTimeE(tTime);
	pcPeerInfo->SetLastSndTimeE2(tTime);
	pcPeerInfo->SetLastSndTimeR(tTime);

	pcPeerInfo->SetRemoteL(ulRemoteL, uiRemotePortL);
	pcPeerInfo->SetRemoteE(ulRemoteE, uiRemotePortE);
	pcPeerInfo->SetRemoteE2(ulRemoteE2, uiRemotePortE2);
	pcPeerInfo->SetRemoteR(m_stRelayPeer);

	//
	pcPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);

	//
	pcPeerInfo->SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);

	//
	m_cPeerCtrlLock.Lock();

	//
	if (NULL == m_cPeerInfoList.Find(pcPeerInfo))
	{
		m_cPeerInfoList.Add(pcPeerInfo);

		//
		m_cPeerCtrlLock.Unlock();

		return TRUE;
	}
	//

	//
	m_cPeerCtrlLock.Unlock();

	//
	m_cPeerInfoList.Deallocate(pcPeerInfo);

	//
	TryCheckRelayAblePeer();

	return FALSE;
}

BOOL
CCtrlPeers::AddPeerEx(const char* pszRemoteL, USHORT uiRemotePortL, const char* pszRemoteE, USHORT uiRemotePortE, const char* pszRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex, int iDatatype)
{
	CPeerInfoNode* pcPeerInfo = m_cPeerInfoList.Allocate();
	if (NULL == pcPeerInfo)
	{
		return FALSE;
	}
	else
	{
		byIndex = pcPeerInfo->GetIndex();
	}

	//
	time_t tTime = time(NULL);

	//
	pcPeerInfo->SetConnFlagL(FALSE);
	pcPeerInfo->SetConnFlagE(FALSE);
	pcPeerInfo->SetConnFlagE2(FALSE);
	pcPeerInfo->SetConnFlagR(TRUE);
	pcPeerInfo->SetRelayAblePeer(FALSE);

	pcPeerInfo->SetLastRcvTimeL(tTime);
	pcPeerInfo->SetLastRcvTimeE(tTime);
	pcPeerInfo->SetLastRcvTimeE2(tTime);
	pcPeerInfo->SetLastRcvTimeR(tTime);

	pcPeerInfo->SetLastSndTimeL(tTime);
	pcPeerInfo->SetLastSndTimeE(tTime);
	pcPeerInfo->SetLastSndTimeE2(tTime);
	pcPeerInfo->SetLastSndTimeR(tTime);

	pcPeerInfo->SetRemoteL(pszRemoteL, uiRemotePortL);
	pcPeerInfo->SetRemoteE(pszRemoteE, uiRemotePortE);
	pcPeerInfo->SetRemoteE2(pszRemoteE2, uiRemotePortE2);
	pcPeerInfo->SetRemoteR(m_stRelayPeer);

	//
	pcPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);

	//
	pcPeerInfo->SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);

	//
	pcPeerInfo->SetDataType(iDatatype);

	//
	m_cPeerCtrlLock.Lock();

	//
	if (NULL == m_cPeerInfoList.Find(pcPeerInfo))
	{
		m_cPeerInfoList.Add(pcPeerInfo);

		//
		m_cPeerCtrlLock.Unlock();

		return TRUE;
	}
	//

	//
	m_cPeerCtrlLock.Unlock();

	//
	m_cPeerInfoList.Deallocate(pcPeerInfo);

	//
	TryCheckRelayAblePeer();

	return FALSE;
}

BOOL
CCtrlPeers::AddPeerEx(ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2, BYTE& byIndex,  int iDatatype)
{
	CPeerInfoNode* pcPeerInfo = m_cPeerInfoList.Allocate();
	if (NULL == pcPeerInfo)
	{
		return FALSE;
	}
	else
	{
		byIndex = pcPeerInfo->GetIndex();
	}

	//
	time_t tTime = time(NULL);

	//
	pcPeerInfo->SetConnFlagL(FALSE);
	pcPeerInfo->SetConnFlagE(FALSE);
	pcPeerInfo->SetConnFlagE2(FALSE);
	pcPeerInfo->SetConnFlagR(TRUE);
	pcPeerInfo->SetRelayAblePeer(FALSE);

	pcPeerInfo->SetLastRcvTimeL(tTime);
	pcPeerInfo->SetLastRcvTimeE(tTime);
	pcPeerInfo->SetLastRcvTimeE2(tTime);
	pcPeerInfo->SetLastRcvTimeR(tTime);

	pcPeerInfo->SetLastSndTimeL(tTime);
	pcPeerInfo->SetLastSndTimeE(tTime);
	pcPeerInfo->SetLastSndTimeE2(tTime);
	pcPeerInfo->SetLastSndTimeR(tTime);

	pcPeerInfo->SetRemoteL(ulRemoteL, uiRemotePortL);
	pcPeerInfo->SetRemoteE(ulRemoteE, uiRemotePortE);
	pcPeerInfo->SetRemoteE2(ulRemoteE2, uiRemotePortE2);
	pcPeerInfo->SetRemoteR(m_stRelayPeer);

	//
	pcPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);

	//
	pcPeerInfo->SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);

	//
	pcPeerInfo->SetDataType(iDatatype);

	//
	m_cPeerCtrlLock.Lock();

	//
	if (NULL == m_cPeerInfoList.Find(pcPeerInfo))
	{
		m_cPeerInfoList.Add(pcPeerInfo);

		//
		m_cPeerCtrlLock.Unlock();

		return TRUE;
	}
	//

	//
	m_cPeerCtrlLock.Unlock();

	//
	m_cPeerInfoList.Deallocate(pcPeerInfo);

	//
	TryCheckRelayAblePeer();

	return FALSE;
}


BOOL
CCtrlPeers::DelPeer(BYTE byIndex)
{
	BOOL bRet;

	//
	m_cPeerCtrlLock.Lock();

	//
	bRet = m_cPeerInfoList.Delete(byIndex);

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL
CCtrlPeers::IsPeerConnection(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE)
{
	BOOL bRet = TRUE;

	//
	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfo;

	//
	cSrchPeerInfo.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_OR_E);
	cSrchPeerInfo.SetRemoteL(stRemote);
	cSrchPeerInfo.SetRemoteE(stRemoteE);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfo));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	if (FALSE == pcSrchPeerInfo->GetConnFlagL() && FALSE == pcSrchPeerInfo->GetConnFlagE() && FALSE == pcSrchPeerInfo->GetConnFlagR())
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL
CCtrlPeers::IsPeerConnection(BYTE byIndex)
{
	BOOL bRet = TRUE;

	//
	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	if (FALSE == pcSrchPeerInfo->GetConnFlagL() && FALSE == pcSrchPeerInfo->GetConnFlagE() && FALSE == pcSrchPeerInfo->GetConnFlagR())
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

void
CCtrlPeers::ResetPeers()
{
	m_cPeerCtrlLock.Lock();

	//
	m_cPeerInfoList.Reset();
	
	//
	m_cPeerCtrlLock.Unlock();
}

BOOL
CCtrlPeers::PeerConnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType, const SOCKADDR_IN& stRemote)
{
	BOOL bRet = TRUE;

	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL != pcSrchPeerInfo)
	{
		if (PU_PEERADDRTYPE_RELAY == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteR().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteR().sin_port)
		{
			pcSrchPeerInfo->SetConnFlagR(TRUE);

			if (FALSE == pcSrchPeerInfo->GetConnFlagL() && FALSE == pcSrchPeerInfo->GetConnFlagE() )
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
		}
		else
		if (PU_PEERADDRTYPE_EXTERNAL == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteE().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteE().sin_port)
		{
			pcSrchPeerInfo->SetConnFlagE(TRUE);

			if (FALSE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
		}
		else
		if (PU_PEERADDRTYPE_EXTERNAL2 == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteE().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteE().sin_port)
		{
			pcSrchPeerInfo->SetConnFlagE2(TRUE);

			if (FALSE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
		}
		else
		if (PU_PEERADDRTYPE_LOCAL == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteL().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteL().sin_port)
		{
			pcSrchPeerInfo->SetConnFlagL(TRUE);

			pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
		}
		else
		{
			bRet = FALSE;
		}
	}
	else
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL
CCtrlPeers::PeerConnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType)
{
	BOOL bRet = TRUE;

	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL != pcSrchPeerInfo)
	{
		if (PU_PEERADDRTYPE_RELAY == enumAddrType)
		{
			pcSrchPeerInfo->SetConnFlagR(TRUE);

			if (FALSE == pcSrchPeerInfo->GetConnFlagL() && FALSE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
		}
		else
		if (PU_PEERADDRTYPE_EXTERNAL == enumAddrType)
		{
			pcSrchPeerInfo->SetConnFlagE(TRUE);

			if (FALSE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
		}
		else
		if (PU_PEERADDRTYPE_EXTERNAL2 == enumAddrType)
		{
			pcSrchPeerInfo->SetConnFlagE2(TRUE);

			if (FALSE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
		}
		else
		if (PU_PEERADDRTYPE_LOCAL == enumAddrType)
		{
			pcSrchPeerInfo->SetConnFlagL(TRUE);

			pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
		}
		else
		{
			bRet = FALSE;
		}
	}
	else
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL
CCtrlPeers::PeerCheckRelayAblePeer(BYTE byIndex)
{
	BOOL bRet = TRUE;

	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL != pcSrchPeerInfo)
	{

		pcSrchPeerInfo->SetRelayAblePeer( TRUE );

	}
	else
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL 
CCtrlPeers::PeerCheckRelayAbleSinglPeer(SOCKADDR_IN stRemote)
{

	//같은 주소가 있다면..
	if(m_cSingPeer.GetRemoteL().sin_addr.S_un.S_addr == stRemote.sin_addr.S_un.S_addr)
	{
		m_cSingPeer.SetRemoteL(stRemote);
		m_cSingPeer.SetConnFlagL(TRUE);
	}
	else if(m_cSingPeer.GetRemoteE2().sin_addr.S_un.S_addr == stRemote.sin_addr.S_un.S_addr)
	{
		//그 주소의 외부 포트를
		m_cSingPeer.SetRemoteE2(stRemote);
		m_cSingPeer.SetConnFlagE2(TRUE);
	}

	return TRUE;
}


BOOL
CCtrlPeers::PeerDisconnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType, const SOCKADDR_IN& stRemote)
{
	BOOL bRet = TRUE;

	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();

		return FALSE;
	}

	if (PU_PEERADDRTYPE_RELAY == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteR().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteR().sin_port)
	{
		pcSrchPeerInfo->SetConnFlagR(FALSE);

		if (PU_PEERADDRTYPE_RELAY == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteE().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteE().sin_port)
	{
		pcSrchPeerInfo->SetConnFlagE(FALSE);

		if (PU_PEERADDRTYPE_EXTERNAL == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagR())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL2 == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteE().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteE().sin_port)
	{
		pcSrchPeerInfo->SetConnFlagE2(FALSE);

		if (PU_PEERADDRTYPE_EXTERNAL2 == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagR())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	if (PU_PEERADDRTYPE_LOCAL == enumAddrType && stRemote.sin_addr.S_un.S_addr == pcSrchPeerInfo->GetRemoteL().sin_addr.S_un.S_addr && stRemote.sin_port == pcSrchPeerInfo->GetRemoteL().sin_port)
	{
		pcSrchPeerInfo->SetConnFlagL(FALSE);

		if (PU_PEERADDRTYPE_LOCAL == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagR())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL
CCtrlPeers::PeerDisconnectCheck(BYTE byIndex, PU_PEERADDRTYPE enumAddrType)
{
	BOOL bRet = TRUE;

	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();

		return FALSE;
	}

	if (PU_PEERADDRTYPE_RELAY == enumAddrType)
	{
		pcSrchPeerInfo->SetConnFlagR(FALSE);

		if (PU_PEERADDRTYPE_RELAY == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL == enumAddrType)
	{
		pcSrchPeerInfo->SetConnFlagE(FALSE);

		if (PU_PEERADDRTYPE_EXTERNAL == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagR())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL2 == enumAddrType)
	{
		pcSrchPeerInfo->SetConnFlagE2(FALSE);

		if (PU_PEERADDRTYPE_EXTERNAL2 == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagL())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_LOCAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagR())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	if (PU_PEERADDRTYPE_LOCAL == enumAddrType)
	{
		pcSrchPeerInfo->SetConnFlagL(FALSE);

		if (PU_PEERADDRTYPE_LOCAL == pcSrchPeerInfo->GetPeerAddrType())
		{
			if (TRUE == pcSrchPeerInfo->GetConnFlagE())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_EXTERNAL2);
			}
			else
			if (TRUE == pcSrchPeerInfo->GetConnFlagR())
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_RELAY);
			}
			else
			{
				pcSrchPeerInfo->SetPeerAddrType(PU_PEERADDRTYPE_NONE);
			}
		}
	}
	else
	{
		bRet = FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return bRet;
}

BOOL
CCtrlPeers::FindPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE)
{
	CPeerInfoNode cSrchPeerInfo;

	//
	cSrchPeerInfo.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);
	cSrchPeerInfo.SetRemoteL(stRemote);
	cSrchPeerInfo.SetRemoteE(stRemoteE);

	//
	m_cPeerCtrlLock.Lock();

	//
	if (NULL == static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfo)))
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

BOOL
CCtrlPeers::UpdatePeerE2(BYTE byIndex, ULONG ulRemoteE2, USHORT uiRemotePortE2)
{
	CPeerInfoNode* pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	pcSrchPeerInfo->SetRemoteE2(ulRemoteE2, uiRemotePortE2);

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

BOOL
CCtrlPeers::FindPeer(BYTE& byIndex, const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE)
{
	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfo;

	//
	cSrchPeerInfo.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);
	cSrchPeerInfo.SetRemoteL(stRemote);
	cSrchPeerInfo.SetRemoteE(stRemoteE);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfo));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	byIndex = pcSrchPeerInfo->GetIndex();

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

BOOL
CCtrlPeers::FindPeer(BYTE& byIndex, ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE)
{
	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfo;

	//
	cSrchPeerInfo.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);
	cSrchPeerInfo.SetRemoteL(ulRemoteL, uiRemotePortL);
	cSrchPeerInfo.SetRemoteE(ulRemoteE, uiRemotePortE);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfo));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	byIndex = pcSrchPeerInfo->GetIndex();

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

BOOL
CCtrlPeers::FindPeer(BYTE byIndex , CPeerInfoNode& cSrchPeerInfo)
{
	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfoTemp;

	//
	cSrchPeerInfoTemp.SetIndex(byIndex);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfoTemp));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	memcpy(&cSrchPeerInfo , pcSrchPeerInfo, sizeof(CPeerInfoNode));

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

BOOL
CCtrlPeers::ModifyPeer(CPeerInfoNode cSrchPeerInfo)
{
	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfoTemp;

	//
	cSrchPeerInfoTemp.SetCompCondition(cSrchPeerInfo.GetCompCondition());
	cSrchPeerInfoTemp.SetIndex(cSrchPeerInfo.GetIndex());
	cSrchPeerInfoTemp.SetRemoteL(cSrchPeerInfo.GetRemoteL());
	cSrchPeerInfoTemp.SetRemoteE(cSrchPeerInfo.GetRemoteE());

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfoTemp));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}
	memcpy(pcSrchPeerInfo , &cSrchPeerInfo, sizeof(CPeerInfoNode));

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

CPeerInfoNode
CCtrlPeers::GetIsConnect( ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE )
{

	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfo;

	//
	cSrchPeerInfo.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);
	cSrchPeerInfo.SetRemoteL(ulRemoteL, uiRemotePortL);
	cSrchPeerInfo.SetRemoteE(ulRemoteE, uiRemotePortE);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfo));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return NULL;
	}

	cSrchPeerInfo = *pcSrchPeerInfo;

	//
	m_cPeerCtrlLock.Unlock();

	return cSrchPeerInfo;

}

BOOL
CCtrlPeers::TryConnectForPeers()
{
	SOCKADDR_IN m_stSelfL;
	ULONG ulSeq;
	ULONG ulIpL;
	USHORT uiPortL;
	ULONG ulIpE;
	USHORT uiPortE;
	PU_PACKET_SYNBODY stSynBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stSynBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stSynBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stSynBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stSynBd.m_abySrcPortE);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stSynBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		ulIpL = pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr;
		uiPortL = pcCurrPeer->GetRemoteL().sin_port;

		B_LONGTOS(ulIpL, stSynBd.m_abyDstIpL);
		B_INTTOS(uiPortL, stSynBd.m_abyDstPortL);

		//
		ulIpE = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
		uiPortE = pcCurrPeer->GetRemoteE().sin_port;

		B_LONGTOS(ulIpE, stSynBd.m_abyDstIpE);
		B_INTTOS(uiPortE, stSynBd.m_abyDstPortE);

		//
		ulSeq = pcCurrPeer->GetIncSeq();

		//
		if (FALSE == SendReqSyn(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
		{
#ifdef _DEBUG
			GetPuLogInstance().Logging("ERROR::SendReqSyn -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteL().sin_addr), ntohs(pcCurrPeer->GetRemoteL().sin_port));
#endif
		}

		// 로컬주소와 외부2주소가 같으면 연결 확인을 할 필요가 없다.
		// 외부1주소는 릴레이서버에서 확인한 주소기 때문에 SendCheckExternalIp()로 확인한다.
		if( !(ulIpL == pcCurrPeer->GetRemoteE2().sin_addr.S_un.S_addr && uiPortL == pcCurrPeer->GetRemoteE2().sin_port) )
		{

			//
			ulSeq = pcCurrPeer->GetIncSeq();

			//
			if (FALSE == SendReqSyn(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
			{
#ifdef _DEBUG
				GetPuLogInstance().Logging("ERROR::SendReqSyn -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteE().sin_addr), ntohs(pcCurrPeer->GetRemoteE().sin_port));
#endif
			}

		}
#ifdef OCF_CON_RELAY
		//
		ulSeq = pcCurrPeer->GetIncSeq();

		//
		if (FALSE == SendRelayData(pcCurrPeer->GetRemoteR(), pcCurrPeer->GetRemoteE(), PU_PKT_HEAD_TYPE_REQSYN, PU_PEERADDRTYPE_RELAY, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
		{
#ifdef _DEBUG
			GetPuLogInstance().Logging("ERROR::SendReqSyn -> RELAY -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteE().sin_addr), ntohs(pcCurrPeer->GetRemoteE().sin_port));
#endif
		}

#endif

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryConnectForLocalPeers()
{
	SOCKADDR_IN m_stSelfL;
	ULONG ulSeq;
	ULONG ulIp;
	USHORT uiPort;
	PU_PACKET_SYNBODY stSynBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stSynBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stSynBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stSynBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stSynBd.m_abySrcPortE);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stSynBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		ulIp = pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteL().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpL);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortL);

		//
		ulIp = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteE().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpE);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortE);

		//
		ulSeq = pcCurrPeer->GetIncSeq();

		//
		if (FALSE == SendReqSyn(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
		{
#ifdef _DEBUG
			GetPuLogInstance().Logging("ERROR::SendReqSyn -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteL().sin_addr), ntohs(pcCurrPeer->GetRemoteL().sin_port));
#endif
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryConnectForExternal2Peers()
{
	SOCKADDR_IN m_stSelfL;
	ULONG ulSeq;
	ULONG ulIp;
	USHORT uiPort;
	PU_PACKET_SYNBODY stSynBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stSynBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stSynBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stSynBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stSynBd.m_abySrcPortE);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stSynBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		ulIp = pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteL().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpL);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortL);

		//
		ulIp = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteE().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpE);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortE);

		//
		ulSeq = pcCurrPeer->GetIncSeq();
		
		//
		if (FALSE == SendReqSyn(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
		{
#ifdef _DEBUG
			GetPuLogInstance().Logging("ERROR::SendReqSyn -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteE().sin_addr), ntohs(pcCurrPeer->GetRemoteE().sin_port));
#endif
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryConnectForRelayPeers()
{
	SOCKADDR_IN m_stSelfL;
	ULONG ulSeq;
	ULONG ulIp;
	USHORT uiPort;
	PU_PACKET_SYNBODY stSynBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stSynBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stSynBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stSynBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stSynBd.m_abySrcPortE);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stSynBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		ulIp = pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteL().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpL);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortL);

		//
		ulIp = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteE().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpE);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortE);

		//
		ulSeq = pcCurrPeer->GetIncSeq();
		
		//
		if (FALSE == SendRelayData(pcCurrPeer->GetRemoteR(), pcCurrPeer->GetRemoteE(), PU_PKT_HEAD_TYPE_REQSYN, PU_PEERADDRTYPE_RELAY, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
		{
#ifdef _DEBUG
			GetPuLogInstance().Logging("ERROR::SendReqSyn -> RELAY -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteE().sin_addr), ntohs(pcCurrPeer->GetRemoteE().sin_port));
#endif
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryDisonnectForPeers()
{
	SOCKADDR_IN m_stSelfL;
	SOCKADDR_IN m_stPeerE;
	ULONG ulSeq;
	ULONG ulIp;
	USHORT uiPort;
	PU_PACKET_FINBODY stFinBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stFinBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stFinBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stFinBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stFinBd.m_abySrcPortE);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stFinBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		ulIp = pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteL().sin_port;

		B_LONGTOS(ulIp, stFinBd.m_abyDstIpL);
		B_INTTOS(uiPort, stFinBd.m_abyDstPortL);

		//
		ulIp = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteE().sin_port;

		B_LONGTOS(ulIp, stFinBd.m_abyDstIpE);
		B_INTTOS(uiPort, stFinBd.m_abyDstPortE);

		//
		if (TRUE == pcCurrPeer->GetConnFlagL())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			SendReqFin(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, sizeof(stFinBd), reinterpret_cast<const BYTE*>(&stFinBd));
		}

		//
		if (TRUE == pcCurrPeer->GetConnFlagE2())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			SendReqFin(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, sizeof(stFinBd), reinterpret_cast<const BYTE*>(&stFinBd));
		}

		//
		if (TRUE == pcCurrPeer->GetConnFlagR())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			m_stPeerE.sin_family = AF_INET;
			m_stPeerE.sin_addr.s_addr = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
			m_stPeerE.sin_port = pcCurrPeer->GetRemoteE().sin_port;

			SendRelayData(pcCurrPeer->GetRemoteR(), m_stPeerE, PU_PKT_HEAD_TYPE_REQFIN, PU_PEERADDRTYPE_RELAY, ulSeq, sizeof(stFinBd), reinterpret_cast<const BYTE*>(&stFinBd));
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryEchoForPeers()
{
	time_t tTime;
	ULONG ulSeq;
	PU_PACKET_ECHOBODY stEchoBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	tTime = time(NULL);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stEchoBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		if (TRUE == pcCurrPeer->GetConnFlagL())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			if (TRUE == SendReqEcho(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, sizeof(stEchoBd), reinterpret_cast<const BYTE*>(&stEchoBd)))
			{
				pcCurrPeer->SetLastSndTimeL(tTime);
			}
		}

		//
		if (TRUE == pcCurrPeer->GetConnFlagE2())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			if (TRUE == SendReqEcho(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, sizeof(stEchoBd), reinterpret_cast<const BYTE*>(&stEchoBd)))
			{
				pcCurrPeer->SetLastSndTimeE2(tTime);
			}
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryDataForPeers(USHORT uiLen, const BYTE* pbyData)
{

	//
	ULONG ulSeq;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	// 릴레이가능 피어를 찾는다.
	vector<CPeerInfoNode*> vectorRelayAblePeer;
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
	while (pcCurrPeer)
	{

		if( TRUE == pcCurrPeer->GetRelayAblePeer() )
		{
			//
			vectorRelayAblePeer.push_back( pcCurrPeer );

		}
	
		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	//
	while (pcCurrPeer)
	{
		//
		// Seq쪽은 추후 사용을 위해서....
		//
		ulSeq = pcCurrPeer->GetIncSeq();

		//
		if (TRUE == pcCurrPeer->GetConnFlagL())
		{
			SendData(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, uiLen, pbyData);
		}
		else if (TRUE == pcCurrPeer->GetConnFlagE2())
		{
			SendData(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, uiLen, pbyData);
		}
		else//if (TRUE == pcCurrPeer->GetConnFlagR())
		{

			// 나와 상대방 모두 릴레이가능가 불가능한 상태라면
			if( 0 != vectorRelayAblePeer.size() )
			{

				//
				CPeerInfoNode* pcRelayAblePeer = vectorRelayAblePeer[rand()%vectorRelayAblePeer.size()];

				// 릴레이가능한 피어가 있으면 호스트 피어로
				SendRelayData(pcRelayAblePeer->GetRemoteE(), pcCurrPeer->GetRemoteE(), PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY_PEER, ulSeq, uiLen, pbyData);
				
			}
			else
			{

				// 릴레이가능한 피어가 없으면 릴레이서버로
				SendRelayData(pcCurrPeer->GetRemoteR(), pcCurrPeer->GetRemoteE(), PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY, ulSeq, uiLen, pbyData);

			}

		}
		//

		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;

}

BOOL
CCtrlPeers::TryDataForPeersEx(USHORT uiLen, const BYTE* pbyData, UINT iDataType)
{

	//
	ULONG ulSeq;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	// 릴레이가능 피어를 찾는다.
	vector<CPeerInfoNode*> vectorRelayAblePeer;

	//
	ULONG ulPeerSize =0;

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
	while (pcCurrPeer)
	{

		if( TRUE == pcCurrPeer->GetRelayAblePeer() )
		{
			//
			vectorRelayAblePeer.push_back( pcCurrPeer );

		}
	
		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	//
	vector<SOCKADDR_IN> vectorRelayDestPeer;

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
	while (pcCurrPeer)
	{
		//포함된 옵션이 있다면..
		if(0 !=  (iDataType & pcCurrPeer->GetDataType()))
		{

			//
			// Seq쪽은 추후 사용을 위해서....
			//
			ulSeq = pcCurrPeer->GetIncSeq();

			//
			if (TRUE == pcCurrPeer->GetConnFlagL())
			{
				SendData(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, uiLen, pbyData);
			}
			else if (TRUE == pcCurrPeer->GetConnFlagE2())
			{
				SendData(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, uiLen, pbyData);
			}
			else//if (TRUE == pcCurrPeer->GetConnFlagR())
			{
				vectorRelayDestPeer.push_back(pcCurrPeer->GetRemoteE());
			}

		}

		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	// 사이즈 구하기
	ULONG ulRelayAbleSize = vectorRelayAblePeer.size();
	ULONG ulRelayDestSize = vectorRelayDestPeer.size();

	// 릴레이 할 대상이 있다면
	if(ulRelayDestSize != 0)
	{

		//릴레이 가능한 피어 && 릴레이서버 우선
		if( 0 != ulRelayAbleSize && !m_bRelayServerFirst)
		{

			//릴레이 피어가 1명 이상일 경우 피어를 균등분활 
			//ex)10명의 대상자와 3명의 릴레이 가능자가 있다면 10/3 = 3.333.... 을 4,4,4(4명묶음식 3명이 릴레이)..
			ULONG ulDivSize = int((float(ulRelayDestSize) / float(ulRelayAbleSize)) + 0.5);

			//
			for(ULONG uli = 0 ; uli < ulRelayAbleSize; uli++)
			{
				//릴레이 할 피어를 선택
				CPeerInfoNode* pcRelayAblePeer = vectorRelayAblePeer[uli];

				//
				vector<SOCKADDR_IN>::iterator itorDest = vectorRelayDestPeer.begin();

				//마지막 사이즈가 균등한 값보다 작다면.. 작은값으로 
				//ex)총 10명중 4,4,4 를 4,4,2, 마지막값은 2로변경 
				ULONG ulSize =  ulRelayDestSize - ( uli * ulDivSize )  >  ulDivSize ? ulDivSize : ulRelayDestSize - (uli * ulDivSize) ;
				
				//벡터의 공간을 미리 설정
				vector<SOCKADDR_IN> vectorDivRelay(ulSize);
				vector<SOCKADDR_IN>::iterator itor = vectorDivRelay.begin();
				
				//필요한 만큼 벡터를 카피
//Migration
				//memcpy( itor._Myptr , itorDest._Myptr + ( uli * ulSize ) , sizeof(SOCKADDR_IN) * ulSize);  
				memcpy( itor._Ptr , itorDest._Ptr + ( uli * ulSize ) , sizeof(SOCKADDR_IN) * ulSize);   

				//
				SendRelayDataEx(pcRelayAblePeer->GetRemoteE(), vectorDivRelay , PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY, ulSeq, uiLen, pbyData);

			}

		}
		//없거나 릴레이서버 우선 이라면 모든 피어를 릴레이서버로 보냅니다
		else
		{

			//
			pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
			
			//
			SendRelayDataEx(pcCurrPeer->GetRemoteR(), vectorRelayDestPeer , PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY, ulSeq, uiLen, pbyData);
		
		} 	

	}

	return TRUE;

}


BOOL
CCtrlPeers::TryDataForPeer(PU_PEER_ADDRESS &stAdd, USHORT uiLen, const BYTE* pbyData)
{

	//
	ULONG ulSeq;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	///
	// 릴레이가능 피어를 찾는다.
	vector<CPeerInfoNode*> vectorRelayAblePeer;
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
	while (pcCurrPeer)
	{

		if( TRUE == pcCurrPeer->GetRelayAblePeer() )
		{
			//
			vectorRelayAblePeer.push_back( pcCurrPeer );

		}

		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	///
	// 피어를 찾는다.
	CPeerInfoNode* pcSrchPeerInfo;
	CPeerInfoNode cSrchPeerInfo;

	//
	cSrchPeerInfo.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);
	cSrchPeerInfo.SetRemoteL(stAdd.ulRemoteL, stAdd.uiRemotePortL);
	cSrchPeerInfo.SetRemoteE(stAdd.ulRemoteE, stAdd.uiRemotePortE);

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(&cSrchPeerInfo));
	if (NULL == pcSrchPeerInfo)
	{
		return FALSE;
	}


	//
	// Seq쪽은 추후 사용을 위해서....
	//
	ulSeq = pcSrchPeerInfo->GetIncSeq();

	//
	if (TRUE == pcSrchPeerInfo->GetConnFlagL())
	{
		SendData(pcSrchPeerInfo->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, uiLen, pbyData);
	}
	else if (TRUE == pcSrchPeerInfo->GetConnFlagE2())
	{
		SendData(pcSrchPeerInfo->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, uiLen, pbyData);
	}
	else//if (TRUE == pcSrchPeerInfo->GetConnFlagR())
	{

		// 나와 상대방 모두 릴레이가능가 불가능한 상태라면
		if( 0 != vectorRelayAblePeer.size() )
		{

			//
			CPeerInfoNode* pcRelayAblePeer = vectorRelayAblePeer[rand()%vectorRelayAblePeer.size()];

			// 릴레이가능한 피어가 있으면 호스트 피어로
			SendRelayData(pcRelayAblePeer->GetRemoteE(), pcSrchPeerInfo->GetRemoteE(), PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY_PEER, ulSeq, uiLen, pbyData);

		}
		else
		{

			// 릴레이가능한 피어가 없으면 릴레이서버로
			SendRelayData(pcSrchPeerInfo->GetRemoteR(), pcSrchPeerInfo->GetRemoteE(), PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY, ulSeq, uiLen, pbyData);

		}

	}

	return TRUE;

}

BOOL
CCtrlPeers::TryDataWithRelayForPeers(USHORT uiLen, const BYTE* pbyData)
{
	ULONG ulSeq;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	//
	while (pcCurrPeer)
	{
		//
		// Seq쪽은 추후 사용을 위해서....
		//
		ulSeq = pcCurrPeer->GetIncSeq();

//		if (TRUE == pcCurrPeer->GetConnFlagR())
		{
			SendRelayData(pcCurrPeer->GetRemoteR(), pcCurrPeer->GetRemoteE(), PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY, ulSeq, uiLen, pbyData);
		}
		//

		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryDataSend( const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff)
{

	SOCKADDR_IN stRemote;
	stRemote.sin_family = AF_INET;
	stRemote.sin_addr.s_addr = inet_addr(pszIP);
	stRemote.sin_port = htons(uiPort);

	SendData( stRemote, PU_PEERADDRTYPE_LOCAL, 0, uiLen, pbyBuff);
	SendData( stRemote, PU_PEERADDRTYPE_EXTERNAL, 0, uiLen, pbyBuff);
	SendData( stRemote, PU_PEERADDRTYPE_EXTERNAL2, 0, uiLen, pbyBuff);

	return TRUE;

}

void
CCtrlPeers::TryCheckPeers(USHORT uiPeriod)
{
	ULONG ulSeq;
	time_t tCurrent = time(NULL);

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	//
	while (pcCurrPeer)
	{
		//
		// Seq쪽은 추후 사용을 위해서....
		//
		ulSeq = pcCurrPeer->GetIncSeq();

		//
		if (TRUE == pcCurrPeer->GetConnFlagL())
		{
			if (uiPeriod < difftime(tCurrent, pcCurrPeer->GetLastRcvTimeL()))
			{
				pcCurrPeer->SetConnFlagL(FALSE);
			}
		}
		else
		if (TRUE == pcCurrPeer->GetConnFlagE2())
		{
			if (uiPeriod < difftime(tCurrent, pcCurrPeer->GetLastRcvTimeE2()))
			{
				pcCurrPeer->SetConnFlagE2(FALSE);
			}
		}
#ifdef OCF_CON_RELAY
		else
		if (TRUE == pcCurrPeer->GetConnFlagR())
		{
			if (uiPeriod < difftime(tCurrent, pcCurrPeer->GetLastRcvTimeR()))
			{
				pcCurrPeer->SetConnFlagR(FALSE);
			}
		}
		//
#endif
		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}
}

///
BOOL
CCtrlPeers::TryCheckRelayAblePeer()
{

	SOCKADDR_IN m_stSelfL;
	ULONG ulSeq;
	ULONG ulIp;
	USHORT uiPort;
	PU_PACKET_SYNBODY stSynBd;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stSynBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stSynBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stSynBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stSynBd.m_abySrcPortE);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stSynBd.m_byDstIndex = pcCurrPeer->GetIndex();

		//
		ulIp = pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteL().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpL);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortL);

		//
		ulIp = pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr;
		uiPort = pcCurrPeer->GetRemoteE().sin_port;

		B_LONGTOS(ulIp, stSynBd.m_abyDstIpE);
		B_INTTOS(uiPort, stSynBd.m_abyDstPortE);

		//
		ulSeq = pcCurrPeer->GetIncSeq();

		//
		if (FALSE == SendReqCheckRelayAblePeer(pcCurrPeer->GetRemoteE(), PU_PEERADDRTYPE_EXTERNAL, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
		{
#ifdef _DEBUG
			GetPuLogInstance().Logging("ERROR::SendReqCheckRelayAblePeer -> [%s], [%d]\n", inet_ntoa(pcCurrPeer->GetRemoteE().sin_addr), ntohs(pcCurrPeer->GetRemoteE().sin_port));
#endif
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;

}

BOOL
CCtrlPeers::GetPeersInfo(ULONG& ulCount, CCtrlMem& cInfo)
{
	PU_PEERINFO stPeerInfo;

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	ulCount = 0;
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		stPeerInfo.m_byIndex = pcCurrPeer->GetIndex();

		stPeerInfo.m_bConnectL = pcCurrPeer->GetConnFlagL();
		stPeerInfo.m_bConnectE = pcCurrPeer->GetConnFlagE();
		stPeerInfo.m_bConnectE2 = pcCurrPeer->GetConnFlagE2();
		stPeerInfo.m_bConnectR = pcCurrPeer->GetConnFlagR();
		stPeerInfo._bRelayAblePeer = pcCurrPeer->GetRelayAblePeer();

		stPeerInfo.m_tLastRcvTimeL = pcCurrPeer->GetLastRcvTimeL();
		stPeerInfo.m_tLastRcvTimeE = pcCurrPeer->GetLastRcvTimeE();
		stPeerInfo.m_tLastRcvTimeE2 = pcCurrPeer->GetLastRcvTimeE2();
		stPeerInfo.m_tLastRcvTimeR = pcCurrPeer->GetLastRcvTimeR();

		stPeerInfo.m_tLastSndTimeL = pcCurrPeer->GetLastSndTimeL();
		stPeerInfo.m_tLastSndTimeE = pcCurrPeer->GetLastSndTimeE();
		stPeerInfo.m_tLastSndTimeE2 = pcCurrPeer->GetLastSndTimeE2();
		stPeerInfo.m_tLastSndTimeR = pcCurrPeer->GetLastSndTimeR();

		memcpy(&stPeerInfo.m_stRemoteL, &pcCurrPeer->GetRemoteL(), sizeof(stPeerInfo.m_stRemoteL));
		memcpy(&stPeerInfo.m_stRemoteE, &pcCurrPeer->GetRemoteE(), sizeof(stPeerInfo.m_stRemoteE));
		memcpy(&stPeerInfo.m_stRemoteE2, &pcCurrPeer->GetRemoteE2(), sizeof(stPeerInfo.m_stRemoteE2));
		memcpy(&stPeerInfo.m_stRemoteR, &pcCurrPeer->GetRemoteR(), sizeof(stPeerInfo.m_stRemoteR));

		stPeerInfo.m_iDataType = pcCurrPeer->GetDataType();
		//
		ulCount++;
		
		//
		if (FALSE == cInfo.In(sizeof(stPeerInfo), &stPeerInfo))
		{
			return FALSE;
		}

		//
		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

BOOL
CCtrlPeers::TryNewPeerForPeers(ULONG ulAddrL, USHORT uiPortL, ULONG ulAddrE, USHORT uiPortE)
{
	ULONG ulSeq;
	PU_PACKET_NEWPEERBODY stNewPeer;

	//
	B_LONGTOS(ulAddrL, stNewPeer.m_abyNewIpL);
	B_INTTOS(uiPortL, stNewPeer.m_abyNewPortL);
	B_LONGTOS(ulAddrE, stNewPeer.m_abyNewIpE);
	B_INTTOS(uiPortE, stNewPeer.m_abyNewPortE);

	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		if (pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr == ulAddrL && pcCurrPeer->GetRemoteL().sin_port == uiPortL && pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr == ulAddrE && pcCurrPeer->GetRemoteE().sin_port == uiPortE)
		{
			pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
			continue;
		}

		if (TRUE == pcCurrPeer->GetConnFlagL())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			SendNewPeer(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, sizeof(stNewPeer), reinterpret_cast<const BYTE*>(&stNewPeer));
		}
		else
		if (TRUE == pcCurrPeer->GetConnFlagE2())
		{
			ulSeq = pcCurrPeer->GetIncSeq();

			SendNewPeer(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, sizeof(stNewPeer), reinterpret_cast<const BYTE*>(&stNewPeer));
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	return TRUE;
}

void
CCtrlPeers::TryNewPeerForEachOther()
{
	//
	CPeerInfoNode* pcCurrPeer;

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	//
	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());

	while (pcCurrPeer)
	{
		TryNewPeerForPeers(
				pcCurrPeer->GetRemoteL().sin_addr.S_un.S_addr, 
				pcCurrPeer->GetRemoteL().sin_port,
				pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr, 
				pcCurrPeer->GetRemoteE().sin_port);

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}
}

void
CCtrlPeers::HandleError(OCFCLIENT_ERROR_CATEGORY enumErrorCategory, LONG ErrorCode, unsigned char chServiceType, ULONG ulLen, const BYTE* pbyBuff)
{
}

BOOL
CCtrlPeers::SendRelayData(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PKT_HEAD_TYPE enumType, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_RELAYHEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RELAYDATA;
	stHd.m_byAddrType = PU_PEERADDRTYPE_RELAY;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	//
	stHd.m_byDstType = enumType;
	stHd.m_byDstAddrType = enumAddrType;
	B_LONGTOS(stDstRemote.sin_addr.S_un.S_addr, stHd.m_abyDstIP);
	B_INTTOS(stDstRemote.sin_port, stHd.m_abyDstPort);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}


//vectorStDstRemote 릴레이 대상자
BOOL
CCtrlPeers::SendRelayDataEx(const SOCKADDR_IN& stRemote, vector<SOCKADDR_IN>& vectorStDstRemote, PU_PKT_HEAD_TYPE enumType, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_RELAYHEAD_EX stHd = {0,};

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RELAYDATA_EX;
	stHd.m_byAddrType = PU_PEERADDRTYPE_RELAY;

	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	//
	stHd.m_byDstType = enumType;
	stHd.m_byDstAddrType = enumAddrType;
	
	ULONG ulDestSize = vectorStDstRemote.size();

	//피어의 최대수가 255정도 일것이기에...
	stHd.m_byDstSize = static_cast<BYTE>(ulDestSize);

	//최대 보낼수있는 피어 수로 미리 지정후
	PU_PACKET_RELAYDESTHEAD_EX stDestHead[PU_PEERS_MAXCOUNT] = {0,};

	for(ULONG ulI = 0; ulI < ulDestSize ; ulI++)
	{
		B_LONGTOS(vectorStDstRemote[ulI].sin_addr.S_un.S_addr, stDestHead[ulI].m_abyDstIP);
		B_INTTOS(vectorStDstRemote[ulI].sin_port, stDestHead[ulI].m_abyDstPort);
	}
	
	//최대 헤더 길이는 피어수 + (피어 ip * 최대)
	BYTE byDestHead[(PU_PACKET_RELAYDESTHEAD_EX_LEN * PU_PEERS_MAXCOUNT)] = {0,};

	//기본헤더 + peer IP List 들을 길이를 구한다
	ULONG ulRelayHeadSize =  PU_PACKET_RELAYHEAD_EX_LEN;
	ULONG ulRelayDestHeadSize = PU_PACKET_RELAYDESTHEAD_EX_LEN * ulDestSize;
	
	//기본헤더
	memcpy( byDestHead , &stHd ,ulRelayHeadSize );

	//피어IP 리스트
	memcpy( &byDestHead[ulRelayHeadSize] , stDestHead ,ulRelayDestHeadSize );

	return CUdpCommClient::Push(stRemote, ulRelayHeadSize+ulRelayDestHeadSize , reinterpret_cast<const BYTE*>(&byDestHead), uiLen, pbyBuff);
	
}

BOOL
CCtrlPeers::SendData(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_DATA;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendReqEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_REQECHO;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendResEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RESECHO;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendReqSyn(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_REQSYN;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendResSyn(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RESSYN;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}


BOOL
CCtrlPeers::SendReqFin(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_REQFIN;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendResFin(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RESFIN;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendReset(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RESET;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendNewPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_NEWPEER;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendReqCheckRelayAblePeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_REQCHECKRELAYABLEPEER;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendResCheckRelayAblePeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RESCHECKRELAYABLEPEER;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::SendReqCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_REQCHECKEXTERNALIP;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(0, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), 0, reinterpret_cast<const BYTE*>(""));
}

BOOL
CCtrlPeers::SendResCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulSeq)
{
	PU_PACKET_HEAD stHd;
	PU_PACKET_CHECKEXTERNALIP stCheckExternalIp;

	B_LONGTOS(stRemote.sin_addr.S_un.S_addr, stCheckExternalIp.m_abyIpE);
	B_INTTOS(stRemote.sin_port, stCheckExternalIp.m_abyPortE);

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = PU_PKT_HEAD_TYPE_RESCHECKEXTERNALIP;
	stHd.m_byAddrType = enumAddrType;
	B_INTTOS(sizeof(stCheckExternalIp), stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), sizeof(stCheckExternalIp), reinterpret_cast<const BYTE*>(&stCheckExternalIp));
}

BOOL
CCtrlPeers::Send(const SOCKADDR_IN& stRemote, BYTE byType, BYTE byAddrType, ULONG ulSeq, USHORT uiLen, const BYTE* pbyBuff)
{
	PU_PACKET_HEAD stHd;

	//
	ULONG ulSerial = GetPuCfgInstance().GetSerial();

	//
	stHd.m_byType = byType;
	stHd.m_byAddrType = byAddrType;
	B_INTTOS(uiLen, stHd.m_abyLen);
	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
	B_LONGTOS(ulSerial, stHd.m_abySerial);

	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
}

BOOL
CCtrlPeers::UpdatePeerRcvTime(BYTE byIndex, PU_PEERADDRTYPE enumAddrType)
{
	time_t tTime;
	CPeerInfoNode* pcSrchPeerInfo;

	//
	tTime = time(NULL);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	if (PU_PEERADDRTYPE_LOCAL == enumAddrType)
	{
		pcSrchPeerInfo->SetLastRcvTimeL(tTime);
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL == enumAddrType)
	{
		pcSrchPeerInfo->SetLastRcvTimeE(tTime);
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL2 == enumAddrType)
	{
		pcSrchPeerInfo->SetLastRcvTimeE2(tTime);
	}
	else
	if (PU_PEERADDRTYPE_RELAY == enumAddrType)
	{
		pcSrchPeerInfo->SetLastRcvTimeR(tTime);
	}

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

BOOL
CCtrlPeers::UpdatePeerAllTime(BYTE byIndex, PU_PEERADDRTYPE enumAddrType)
{
	time_t tTime;
	CPeerInfoNode* pcSrchPeerInfo;

	//
	tTime = time(NULL);

	//
	m_cPeerCtrlLock.Lock();

	//
	pcSrchPeerInfo = static_cast<CPeerInfoNode*>(m_cPeerInfoList.Find(byIndex));
	if (NULL == pcSrchPeerInfo)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	if (PU_PEERADDRTYPE_LOCAL == enumAddrType)
	{
		pcSrchPeerInfo->SetLastSndTimeL(tTime);
		pcSrchPeerInfo->SetLastRcvTimeL(tTime);
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL == enumAddrType)
	{
		pcSrchPeerInfo->SetLastSndTimeE(tTime);
		pcSrchPeerInfo->SetLastRcvTimeE(tTime);
	}
	else
	if (PU_PEERADDRTYPE_EXTERNAL2 == enumAddrType)
	{
		pcSrchPeerInfo->SetLastSndTimeE2(tTime);
		pcSrchPeerInfo->SetLastRcvTimeE2(tTime);
	}
	else
	if (PU_PEERADDRTYPE_RELAY == enumAddrType)
	{
		pcSrchPeerInfo->SetLastSndTimeR(tTime);
		pcSrchPeerInfo->SetLastRcvTimeR(tTime);
	}

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

void
CCtrlPeers::Dispatch(BOOL bForce)
{
	CUdpPacketNode* pcPktNode;

	if (FALSE == bForce)
	{
		//
		m_cRcvCtrlLock.Lock();

		//
		if (TRUE == m_cRcvPktList.IsEmpty())
		{
			m_cRcvCtrlLock.Unlock();
			return;
		}
		//

		//
		// OnDataPeer에 의해 Queue된 수신 패킷을 앞쪽에서 가져옵니다.
		//
		pcPktNode = static_cast<CUdpPacketNode*>(m_cRcvPktList.DelBeginWithLive());

		//
		m_cRcvCtrlLock.Unlock();

		//
		// 이벤트함수를 호출합니다.
		// OnDataPeer하고 다른 점은 OnDataPeer는 통신 쓰레드에서 호출되고 OnQueueDataPeer는 메인쓰레드(?) 루프에서 
		// 호출되는 Dispatch함수와 같은 쓰레드에서 호출됨.
		//
		OnQueueDataPeer(pcPktNode->GetRemoteAddress(), static_cast<PU_PEERADDRTYPE>(reinterpret_cast<const PU_PACKET_HEAD*>(pcPktNode->GetRawBuff())->m_byAddrType), pcPktNode->GetRawLen(), pcPktNode->GetRawBuff());
		//

		//
		m_cRcvPktPool.Deallocate(pcPktNode);
	}
	else
	{
		while (TRUE)
		{
			//
			m_cRcvCtrlLock.Lock();

			//
			if (TRUE == m_cRcvPktList.IsEmpty())
			{
				m_cRcvCtrlLock.Unlock();
				return;
			}
			//

			//
			// OnDataPeer에 의해 Queue된 수신 패킷을 앞쪽에서 가져옵니다.
			//
			pcPktNode = static_cast<CUdpPacketNode*>(m_cRcvPktList.DelBeginWithLive());

			//
			m_cRcvCtrlLock.Unlock();

			//
			// 이벤트함수를 호출합니다.
			// OnDataPeer하고 다른 점은 OnDataPeer는 통신 쓰레드에서 호출되고 OnQueueDataPeer는 메인쓰레드(?) 루프에서 
			// 호출되는 Dispatch함수와 같은 쓰레드에서 호출됨.
			//
			OnQueueDataPeer(pcPktNode->GetRemoteAddress(), static_cast<PU_PEERADDRTYPE>(reinterpret_cast<const PU_PACKET_HEAD*>(pcPktNode->GetRawBuff())->m_byAddrType), pcPktNode->GetRawLen(), pcPktNode->GetRawBuff());
			//

			//
			m_cRcvPktPool.Deallocate(pcPktNode);
		}
	}
}

void
CCtrlPeers::ClearInQueue()
{
	CUdpPacketNode* pcPktNode;

	//
	CSmartLock cLock(m_cRcvCtrlLock);

	//
	while (FALSE == m_cRcvPktList.IsEmpty())
	{
		pcPktNode = static_cast<CUdpPacketNode*>(m_cRcvPktList.DelBeginWithLive());

		//
		m_cRcvPktPool.Deallocate(pcPktNode);
	}
}

BOOL
CCtrlPeers::CheckReqConnect(ULONG ulLocalE, USHORT uiLocalPortE)
{
	if (ulLocalE == m_stSelfExternal.sin_addr.S_un.S_addr && uiLocalPortE == m_stSelfExternal.sin_port)
	{
		return TRUE;
	}
	return FALSE;
}

void
CCtrlPeers::PktHandleReqEcho(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	OnReqEcho(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

	//
	SendResEcho(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), B_STOLONG(pstPuHd->m_abySeqAck), static_cast<USHORT>(ulLen - PU_PACKET_HEAD_LEN), reinterpret_cast<const BYTE*>(pstPuHd + 1));
}

void
CCtrlPeers::PktHandleResEcho(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (PU_PACKET_ECHOBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	//
	const PU_PACKET_ECHOBODY* pstEchoBd = reinterpret_cast<const PU_PACKET_ECHOBODY*>(pstPuHd + 1);

	//
	UpdatePeerRcvTime(pstEchoBd->m_byDstIndex, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType));

	//
	OnResEcho(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
}

// 다른 피어에서 연결요청 수신함 (다른피어 -> 내피어)
void
CCtrlPeers::PktHandleReqSyn(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{

	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (PU_PACKET_SYNBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);
	
	//
	if (PU_PEERADDRTYPE_RELAY == pstPuHd->m_byAddrType)
	{
		SOCKADDR_IN stRemoteE;

		stRemoteE.sin_family = AF_INET;
		stRemoteE.sin_addr.s_addr = B_STOLONG(pstSynBd->m_abySrcIpE);
		stRemoteE.sin_port = B_STOINT(pstSynBd->m_abySrcPortE);

		//
		OnReqRelayConnectPeer(stRemote, stRemoteE, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

		//
		SendRelayData(stRemote, stRemoteE, PU_PKT_HEAD_TYPE_RESSYN, PU_PEERADDRTYPE_RELAY, B_STOLONG(pstPuHd->m_abySeqAck), static_cast<USHORT>(ulLen - PU_PACKET_HEAD_LEN), reinterpret_cast<const BYTE*>(pstPuHd + 1));
	}
	else
	{

 		const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);

		SOCKADDR_IN stRemoteSrcL;
		SOCKADDR_IN stRemoteSrcE;

		//
		stRemoteSrcE.sin_family = AF_INET;
		stRemoteSrcE.sin_addr.S_un.S_addr = B_STOLONG(pstSynBd->m_abySrcIpE);
		stRemoteSrcE.sin_port = B_STOINT(pstSynBd->m_abySrcPortE);

		// e2포트 값이 변하면 갱신
		if( (stRemoteSrcE.sin_port != stRemote.sin_port ) )
		{

			// 로컬
			stRemoteSrcL.sin_family = AF_INET;
			stRemoteSrcL.sin_addr.S_un.S_addr = B_STOLONG(pstSynBd->m_abySrcIpL);
			stRemoteSrcL.sin_port = B_STOINT(pstSynBd->m_abySrcPortL);

			//
			BYTE byIndex;
			if( TRUE == FindPeer( byIndex, stRemoteSrcL, stRemoteSrcE ) )
			{
				
				UpdatePeerE2( byIndex, stRemote.sin_addr.S_un.S_addr, stRemote.sin_port );

			}

		}

		if (TRUE == CheckReqConnect(B_STOLONG(pstSynBd->m_abyDstIpE), B_STOINT(pstSynBd->m_abyDstPortE)))
		{

			OnReqConnectPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

			//
			SendResSyn(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), B_STOLONG(pstPuHd->m_abySeqAck), static_cast<USHORT>(ulLen - PU_PACKET_HEAD_LEN), reinterpret_cast<const BYTE*>(pstPuHd + 1));
		}
	}
}

// 다른 피어에서 연결완료 수신함 (내피어 -> 다른피어 -> 내피어)
void
CCtrlPeers::PktHandleResSyn(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (PU_PACKET_SYNBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	//
	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);

	//
	UpdatePeerAllTime(pstSynBd->m_byDstIndex, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType));

	//
	if (PU_PEERADDRTYPE_RELAY == pstPuHd->m_byAddrType)
	{
		SOCKADDR_IN stRemoteE;

		stRemoteE.sin_family = AF_INET;
		stRemoteE.sin_addr.s_addr = B_STOLONG(pstSynBd->m_abySrcIpE);
		stRemoteE.sin_port = B_STOINT(pstSynBd->m_abySrcPortE);

		OnResRelayConnectPeer(stRemote, stRemoteE, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
	}
	else
	{

		OnResConnectPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
	}
}

// 
void
CCtrlPeers::PktHandleReqCheckRelayAblePeer(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{

	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
		return;
	}

	//
	if (PU_PACKET_SYNBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);

	//
	if (TRUE == CheckReqConnect(B_STOLONG(pstSynBd->m_abyDstIpE), B_STOINT(pstSynBd->m_abyDstPortE)))
	{

		// 릴레이 가능
		SetRelayAblePeerFlag( TRUE );

		//
		OnReqCheckRelayAblePeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

		//
		BYTE *pTemp = NULL;
		SendResCheckRelayAblePeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), B_STOLONG(pstPuHd->m_abySeqAck), static_cast<USHORT>(0), pTemp);
	}

	PeerCheckRelayAbleSinglPeer(stRemote);

#ifdef _DEBUG
	GetPuLogInstance().Logging("ReqCheckRelayAbleSinglPeer IP:%s PORT:%d  \n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port) );

#endif

}

void
CCtrlPeers::PktHandleResCheckRelayAblePeer(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{

	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
		return;
	}

	//
 	if (0 != B_STOINT(pstPuHd->m_abyLen))
 	{
 		return;
 	}

	//
	OnResCheckRelayAblePeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

}

void
CCtrlPeers::PktHandleReqFin(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (PU_PACKET_FINBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	const PU_PACKET_FINBODY* pstFinBd = reinterpret_cast<const PU_PACKET_FINBODY*>(pstPuHd + 1);
	
	//
	if (PU_PEERADDRTYPE_RELAY == pstPuHd->m_byAddrType)
	{
		SOCKADDR_IN stRemoteE;

		stRemoteE.sin_family = AF_INET;
		stRemoteE.sin_addr.s_addr = B_STOLONG(pstFinBd->m_abySrcIpE);
		stRemoteE.sin_port = B_STOINT(pstFinBd->m_abySrcPortE);

		//
		OnReqRelayDisconnectPeer(stRemote, stRemoteE, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

		//
		SendRelayData(stRemote, stRemoteE, PU_PKT_HEAD_TYPE_RESFIN, PU_PEERADDRTYPE_RELAY, B_STOLONG(pstPuHd->m_abySeqAck), static_cast<USHORT>(ulLen - PU_PACKET_HEAD_LEN), reinterpret_cast<const BYTE*>(pstPuHd + 1));
	}
	else
	{
		OnReqDisconnectPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);

		//
		SendResFin(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), B_STOLONG(pstPuHd->m_abySeqAck), static_cast<USHORT>(ulLen - PU_PACKET_HEAD_LEN), reinterpret_cast<const BYTE*>(pstPuHd + 1));
	}
}

void
CCtrlPeers::PktHandleResFin(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (PU_PACKET_FINBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	const PU_PACKET_FINBODY* pstFinBd = reinterpret_cast<const PU_PACKET_FINBODY*>(pstPuHd + 1);
	
	//
	if (PU_PEERADDRTYPE_RELAY == pstPuHd->m_byAddrType)
	{
		SOCKADDR_IN stRemoteE;

		stRemoteE.sin_family = AF_INET;
		stRemoteE.sin_addr.s_addr = B_STOLONG(pstFinBd->m_abySrcIpE);
		stRemoteE.sin_port = B_STOINT(pstFinBd->m_abySrcPortE);

		OnResRelayDisconnectPeer(stRemote, stRemoteE, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
	}
	else
	{
		OnResDisconnectPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
	}
}

void
CCtrlPeers::PktHandleReset(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

//	OnResetPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
}

void
CCtrlPeers::PktHandleData(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (FALSE == m_bRcvPktQueue)
	{

		GetPuLogInstance().Logging( "PktHandleData [%d, %d]\n", static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen );
 		OnDataPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
		return;
	}

	//
	CUdpPacketNode* pcPktNode = m_cRcvPktPool.Allocate();
	if (NULL == pcPktNode)
	{
		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0, ulLen, pbyBuff);
		return;
	}

	//
	if (FALSE == pcPktNode->Init(stRemote, ulLen, pbyBuff))
	{
		m_cRcvPktPool.Deallocate(pcPktNode);

		HandleError(OCFCLIENT_ERROR_USERDEF, OCFCLIENT_ERROR_USERDEF_CATEGORY_MEMORY, 0, ulLen, pbyBuff);
		return;
	}

	//
	m_cRcvCtrlLock.Lock();

	//
	m_cRcvPktList.Add(pcPktNode);

	//
	m_cRcvCtrlLock.Unlock();
}

void
CCtrlPeers::PktHandleRelayData(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	if (ulLen < PU_PACKET_RELAYHEAD_LEN)
	{
		return;
	}

	//
	const PU_PACKET_RELAYHEAD* pstPuRelayHd = reinterpret_cast<const PU_PACKET_RELAYHEAD*>(pbyBuff);

	//
	if (PU_PKT_HEAD_TYPE_RELAYDATA == pstPuRelayHd->m_byDstType)
	{
		return;
	}

	//
	SOCKADDR_IN stDstRemote = { 0, };

	stDstRemote.sin_family = AF_INET;
	stDstRemote.sin_addr.s_addr = B_STOLONG(pstPuRelayHd->m_abyDstIP);
	stDstRemote.sin_port = B_STOINT(pstPuRelayHd->m_abyDstPort);

	//
	CSmartLock cLock(m_cPeerCtrlLock);

	// EX2를 찾아서 보냄
	CPeerInfoNode* pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
	while (pcCurrPeer)
	{

		if( stDstRemote.sin_addr.S_un.S_addr == pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr &&
			stDstRemote.sin_port == pcCurrPeer->GetRemoteE().sin_port )
		{

			stDstRemote = pcCurrPeer->GetRemoteE2();
			break;
		}

		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	}

	//
	OnRelayDataPeer(stRemote, stDstRemote, static_cast<PU_PEERADDRTYPE>(pstPuRelayHd->m_byAddrType), ulLen, pbyBuff);

	//
	Send(stDstRemote, pstPuRelayHd->m_byDstType, pstPuRelayHd->m_byDstAddrType, B_STOLONG(pstPuRelayHd->m_abySeqAck), B_STOINT(pstPuRelayHd->m_abyLen), reinterpret_cast<const BYTE*>(pstPuRelayHd + 1));


	// 피어호스트 릴레이
	GetPuLogInstance().Logging( "PktHandleRelayData - %d, %d, %d, %s, %d, %s, %d\n",
		pstPuRelayHd->m_byDstType, pstPuRelayHd->m_byDstAddrType, B_STOINT(pstPuRelayHd->m_abyLen),
		inet_ntoa(stRemote.sin_addr), htons(stRemote.sin_port),
		inet_ntoa(stDstRemote.sin_addr), htons(stDstRemote.sin_port) );

}

void
CCtrlPeers::PktHandleRelayDataEx(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (ulLen < PU_PACKET_RELAYHEAD_LEN)
	{
		return;
	}

	//
	const PU_PACKET_RELAYHEAD_EX* pstPuRelayHd = reinterpret_cast<const PU_PACKET_RELAYHEAD_EX*>(pbyBuff);

	//
	if (PU_PKT_HEAD_TYPE_RELAYDATA == pstPuRelayHd->m_byDstType)
	{
		return;
	}

	//페킷구성# //PU_PACKET_RELAYDESTHEAD_EX //PU_PACKET_RELAYDESTHEAD_EX * size //Data //
	ULONG ulRelayDestHeadSize = PU_PACKET_RELAYDESTHEAD_EX_LEN;
	ULONG ulHeadSize = PU_PACKET_RELAYHEAD_EX_LEN;

	//페킷 대상 사이즈
	ULONG ulDestArraySize = static_cast<BYTE>(pstPuRelayHd->m_byDstSize);

	//
	PU_PACKET_RELAYDESTHEAD_EX stDestHead[PU_PEERS_MAXCOUNT] = {0,};

	//릴레이 대상 헤더를 pbyBuff로부터 잘라낸다
	for(ULONG uli = 0; uli < ulDestArraySize ; uli++)
	{
		memcpy( &stDestHead[uli] ,  pbyBuff + (ulHeadSize + (ulRelayDestHeadSize * uli) ) , ulRelayDestHeadSize );
	}

	//헤더 뒤에 데이터 길이를 구하기 위해
	ULONG ulStartRelayPktSize = (ulRelayDestHeadSize * ulDestArraySize) + ulHeadSize;
	ULONG ulRelayPktSize = ulLen - ulStartRelayPktSize;

	for(ULONG uli = 0 ; uli < ulDestArraySize ; uli++)
	{

		//
		SOCKADDR_IN stDstRemote = { 0, };

		stDstRemote.sin_family = AF_INET;
		stDstRemote.sin_addr.s_addr = B_STOLONG(stDestHead[uli].m_abyDstIP);
		stDstRemote.sin_port = B_STOINT(stDestHead[uli].m_abyDstPort);

		//
		CSmartLock cLock(m_cPeerCtrlLock);

		// EX2를 찾아서 보냄
		CPeerInfoNode* pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
		while (pcCurrPeer)
		{

			if( stDstRemote.sin_addr.S_un.S_addr == pcCurrPeer->GetRemoteE().sin_addr.S_un.S_addr &&
				stDstRemote.sin_port == pcCurrPeer->GetRemoteE().sin_port )
			{

				stDstRemote = pcCurrPeer->GetRemoteE2();
				break;
			}

			pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
		}

		//
		OnRelayDataPeerEx(stRemote, stDstRemote, static_cast<PU_PEERADDRTYPE>(pstPuRelayHd->m_byAddrType), ulRelayPktSize, pbyBuff );

		//
		Send(stDstRemote, pstPuRelayHd->m_byDstType, pstPuRelayHd->m_byDstAddrType, B_STOLONG(pstPuRelayHd->m_abySeqAck), B_STOINT(pstPuRelayHd->m_abyLen), reinterpret_cast<const BYTE*>(pbyBuff+ ulStartRelayPktSize));

		// 피어호스트 릴레이
		GetPuLogInstance().Logging( "PktHandleRelayData - %d, %d, %d, %s, %d, %s, %d\n",
			pstPuRelayHd->m_byDstType, pstPuRelayHd->m_byDstAddrType, B_STOINT(pstPuRelayHd->m_abyLen),
			inet_ntoa(stRemote.sin_addr), htons(stRemote.sin_port),
			inet_ntoa(stDstRemote.sin_addr), htons(stDstRemote.sin_port) );
	}

}

void
CCtrlPeers::PktHandleNewPeer(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (B_STOLONG(pstPuHd->m_abySerial) != GetPuCfgInstance().GetSerial())
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("Pop, Address [%s], [%d], Serial[%d] Invalid\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), B_STOLONG(pstPuHd->m_abySerial));
#endif
		return;
	}

	//
	if (PU_PACKET_NEWPEERBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	OnNewPeer(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), ulLen, pbyBuff);
}

void
CCtrlPeers::PktHandleReqCheckExternalIp(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	OnReqCheckExternalIp(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType));
	SendResCheckExternalIp(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), B_STOLONG(pstPuHd->m_abySeqAck));
}

void
CCtrlPeers::PktHandleResCheckExternalIp(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (sizeof(PU_PACKET_CHECKEXTERNALIP) != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	//
	const PU_PACKET_CHECKEXTERNALIP* pstCheckExternalIp = reinterpret_cast<const PU_PACKET_CHECKEXTERNALIP*>(pstPuHd + 1);

	//
	OnResCheckExternalIp(stRemote, static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType), B_STOLONG(pstCheckExternalIp->m_abyIpE), B_STOINT(pstCheckExternalIp->m_abyPortE));

	//
	SetSelfExternalInfo(B_STOLONG(pstCheckExternalIp->m_abyIpE), B_STOINT(pstCheckExternalIp->m_abyPortE));
}

BOOL 
CCtrlPeers::ModifyMultiMediaPeers(UINT uiMask, BOOL bAndOR)
{
	m_cPeerCtrlLock.Lock();

	//
	ULONG ulCount;

	const PU_PEERINFO* pstPeerInfo;

	CCtrlMem cInfo;

	if (FALSE == GetPeersInfo(ulCount, cInfo))
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	if (0 == ulCount)
	{
		m_cPeerCtrlLock.Unlock();
		return FALSE;
	}

	//
	pstPeerInfo = reinterpret_cast<const PU_PEERINFO*>(cInfo.Reference());

	//
	for (ULONG ulCnt = 0; ulCnt < ulCount; ulCnt++)
	{
		//
		CPeerInfoNode cPeerInfo;
		FindPeer(  pstPeerInfo->m_byIndex , cPeerInfo  );
		
		//
		UINT uiProcess = cPeerInfo.GetDataType();
		
		//
		uiProcess = bAndOR ? uiProcess & uiMask : uiProcess | uiMask;

		//
		cPeerInfo.SetDataType( uiProcess);

		//
		ModifyPeer( cPeerInfo);

		//
		pstPeerInfo++;
	}

	//
	m_cPeerCtrlLock.Unlock();

	return TRUE;
}

//해당 피어로의 속도를 체크합니다
// BOOL
// CCtrlPeers::SendSpeedChk(SOCKADDR_IN& stRemote)
// {
// 	PU_PACKET_HEAD stHd;
// 	stHd.m_byType = PU_PKT_HEAD_TYPE_SENDSPEED;
// 	stHd.m_byAddrType = PU_PEERADDRTYPE_RELAY;
// 
// 	//
// 	ULONG ulSerial = GetPuCfgInstance().GetSerial();
// 
// 	//
// 	B_INTTOS(uiLen, stHd.m_abyLen);
// 	B_LONGTOS(ulSeq, stHd.m_abySeqAck);
// 	B_LONGTOS(ulSerial, stHd.m_abySerial);
// 
// 	BYTE pbyBuff[100000] = {0,};
// 
// 	UINT uiLen = 100000;
// 
// 	return CUdpCommClient::Push(stRemote, sizeof(stHd), reinterpret_cast<const BYTE*>(&stHd), uiLen, pbyBuff);
// 
// }

// void
// CCtrlPeers::PktHandleSendSpeed(const SOCKADDR_IN& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
// {
//}
BOOL 
CCtrlPeers::SetSinglePeer(ULONG ulRemoteL, USHORT uiRemotePortL, ULONG ulRemoteE, USHORT uiRemotePortE, ULONG ulRemoteE2, USHORT uiRemotePortE2,  int iDatatype)
{

	//
	time_t tTime = time(NULL);

	//
	m_cSingPeer.SetConnFlagL(FALSE);
	m_cSingPeer.SetConnFlagE(FALSE);
	m_cSingPeer.SetConnFlagE2(FALSE);
	m_cSingPeer.SetConnFlagR(TRUE);
	m_cSingPeer.SetRelayAblePeer(FALSE);

	m_cSingPeer.SetLastRcvTimeL(tTime);
	m_cSingPeer.SetLastRcvTimeE(tTime);
	m_cSingPeer.SetLastRcvTimeE2(tTime);
	m_cSingPeer.SetLastRcvTimeR(tTime);

	m_cSingPeer.SetLastSndTimeL(tTime);
	m_cSingPeer.SetLastSndTimeE(tTime);
	m_cSingPeer.SetLastSndTimeE2(tTime);
	m_cSingPeer.SetLastSndTimeR(tTime);

	m_cSingPeer.SetRemoteL(ulRemoteL, uiRemotePortL);
	m_cSingPeer.SetRemoteE(ulRemoteE, uiRemotePortE);
	m_cSingPeer.SetRemoteE2(ulRemoteE2, uiRemotePortE2);
	m_cSingPeer.SetRemoteR(m_stRelayPeer);

	//
	m_cSingPeer.SetPeerAddrType(PU_PEERADDRTYPE_NONE);

	//
	m_cSingPeer.SetCompCondition(PU_COMPCONDITION_ADDRESS_L_AND_E);

	//
	m_cSingPeer.SetDataType(iDatatype);



	return TRUE;
}

BOOL
CCtrlPeers::TryCheckRelayAbleSinglePeer()
{

	SOCKADDR_IN m_stSelfL;
	ULONG ulSeq = 0;
	ULONG ulIp;
	USHORT uiPort;
	PU_PACKET_SYNBODY stSynBd;


	//
	if (FALSE == GetLocalAddrInfo(m_stSelfL))
	{
		return FALSE;
	}

	//
	B_LONGTOS(m_stSelfL.sin_addr.S_un.S_addr, stSynBd.m_abySrcIpL);
	B_INTTOS(m_stSelfL.sin_port, stSynBd.m_abySrcPortL);

	B_LONGTOS(m_stSelfExternal.sin_addr.s_addr, stSynBd.m_abySrcIpE);
	B_INTTOS(m_stSelfExternal.sin_port, stSynBd.m_abySrcPortE);

	//
	ulIp = m_cSingPeer.GetRemoteL().sin_addr.S_un.S_addr;
	uiPort = m_cSingPeer.GetRemoteL().sin_port;

	B_LONGTOS(ulIp, stSynBd.m_abyDstIpL);
	B_INTTOS(uiPort, stSynBd.m_abyDstPortL);

	//
	ulIp = m_cSingPeer.GetRemoteE().sin_addr.S_un.S_addr;
	uiPort = m_cSingPeer.GetRemoteE().sin_port;

	B_LONGTOS(ulIp, stSynBd.m_abyDstIpE);
	B_INTTOS(uiPort, stSynBd.m_abyDstPortE);

	//
	//ulSeq = pcCurrPeer->GetIncSeq();

	//
	if (FALSE == SendReqCheckRelayAblePeer(m_cSingPeer.GetRemoteE(), PU_PEERADDRTYPE_EXTERNAL, ulSeq, sizeof(stSynBd), reinterpret_cast<const BYTE*>(&stSynBd)))
	{
#ifdef _DEBUG
		GetPuLogInstance().Logging("ERROR::SendReqCheckRelayAblePeer -> [%s], [%d]\n", inet_ntoa(m_cSingPeer.GetRemoteE().sin_addr), ntohs(m_cSingPeer.GetRemoteE().sin_port));
#endif
	}


	return TRUE;

}

BOOL
CCtrlPeers::TryDataForSinglePeer( USHORT uiLen, const BYTE* pbyData )
{

	//
 	ULONG ulSeq;

	//
	// Seq쪽은 추후 사용을 위해서....
	//
	ulSeq = m_cSingPeer.GetIncSeq();

	//
	if (TRUE == m_cSingPeer.GetConnFlagL())
	{

		SendData(m_cSingPeer.GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, uiLen, pbyData);

	}
	else if (TRUE == m_cSingPeer.GetConnFlagE2())
	{

		SendData(m_cSingPeer.GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, uiLen, pbyData);

	}
	else//if (TRUE == pcSrchPeerInfo.GetConnFlagR())
	{

		// 릴레이가능한 피어가 없으면 릴레이서버로
		SendRelayData(m_cSingPeer.GetRemoteR(), m_cSingPeer.GetRemoteE(), PU_PKT_HEAD_TYPE_DATA, PU_PEERADDRTYPE_RELAY, ulSeq, uiLen, pbyData);
	
	}

	return TRUE;	
}



BOOL
CCtrlPeers::SendSpeedChk(const char* pszIP, USHORT uiPort)
{
	//	const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff

	SOCKADDR_IN stRemote;
	stRemote.sin_family = AF_INET;
	stRemote.sin_addr.s_addr = inet_addr(pszIP);
	stRemote.sin_port = htons(uiPort);

	BYTE pbyBuff[10000] = {0,};	
	UINT uiLen = 10000;


	//SendReqEcho(stRemote, PU_PEERADDRTYPE_EXTERNAL2, 0, uiLen, pbyBuff);
	SendData(stRemote, PU_PEERADDRTYPE_EXTERNAL2, 0, uiLen, pbyBuff);

	return true;

	// 	SendData( stRemote, PU_PEERADDRTYPE_LOCAL, 0, uiLen, pbyBuff);
	// 	SendData( stRemote, PU_PEERADDRTYPE_EXTERNAL, 0, uiLen, pbyBuff);
	// 	SendData( stRemote, PU_PEERADDRTYPE_EXTERNAL2, 0, uiLen, pbyBuff);

	// 	time_t tTime;
	// 	ULONG ulSeq;
	// 	PU_PACKET_ECHOBODY stEchoBd;
	// 
	// 	//
	// 	CPeerInfoNode* pcCurrPeer;
	// 
	// 	//
	// 	tTime = time(NULL);
	// 
	// 	//
	// 	CSmartLock cLock(m_cPeerCtrlLock);
	// 
	// 	//
	// 	pcCurrPeer = static_cast<CPeerInfoNode*>(m_cPeerInfoList.GetBegin());
	// 
	// 	while (pcCurrPeer)
	// 	{
	// 		stEchoBd.m_byDstIndex = pcCurrPeer->GetIndex();
	// 
	// 		//
	// 		if (TRUE == pcCurrPeer->GetConnFlagL())
	// 		{
	// 			ulSeq = pcCurrPeer->GetIncSeq();
	// 
	// 			if (TRUE == SendReqEcho(pcCurrPeer->GetRemoteL(), PU_PEERADDRTYPE_LOCAL, ulSeq, sizeof(stEchoBd), reinterpret_cast<const BYTE*>(&stEchoBd)))
	// 			{
	// 				pcCurrPeer->SetLastSndTimeL(tTime);
	// 			}
	// 		}
	// 
	// 		//
	// 		if (TRUE == pcCurrPeer->GetConnFlagE2())
	// 		{
	// 			ulSeq = pcCurrPeer->GetIncSeq();
	// 
	// 			if (TRUE == SendReqEcho(pcCurrPeer->GetRemoteE2(), PU_PEERADDRTYPE_EXTERNAL2, ulSeq, sizeof(stEchoBd), reinterpret_cast<const BYTE*>(&stEchoBd)))
	// 			{
	// 				pcCurrPeer->SetLastSndTimeE2(tTime);
	// 			}
	// 		}
	// 
	// 		pcCurrPeer = static_cast<CPeerInfoNode*>(pcCurrPeer->GetNext());
	// 	}
}
