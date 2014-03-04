#include "PuPeers.h"

//
CPuPeers::CPuPeers()
{
}

CPuPeers::~CPuPeers()
{
}

void
CPuPeers::OnReqEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
//	m_pcParent->OnReqEcho(stRemote, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnResEcho(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
//	m_pcParent->OnResEcho(stRemote, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnResConnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);
	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);

	PeerConnectCheck(pstSynBd->m_byDstIndex, enumAddrType);

//	m_pcParent->OnResConnectPeer(stRemote, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnReqRelayConnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);
	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);

	//
//	m_pcParent->OnReqRelayConnectPeer(stRemote, stRemoteE, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnResRelayConnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);
	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);

	PeerConnectCheck(pstSynBd->m_byDstIndex, enumAddrType);

}

void
CPuPeers::OnResDisconnectPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);
	const PU_PACKET_FINBODY* pstFinBd = reinterpret_cast<const PU_PACKET_FINBODY*>(pstPuHd + 1);
	
	PeerDisconnectCheck(pstFinBd->m_byDstIndex, enumAddrType);

//	m_pcParent->OnResDisconnectPeer(stRemote, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnReqRelayDisconnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
//	m_pcParent->OnReqRelayDisconnectPeer(stRemote, stRemoteE, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnResRelayDisconnectPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stRemoteE, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);
	const PU_PACKET_FINBODY* pstFinBd = reinterpret_cast<const PU_PACKET_FINBODY*>(pstPuHd + 1);

	PeerDisconnectCheck(pstFinBd->m_byDstIndex, enumAddrType);

}

void
CPuPeers::OnRelayDataPeer(const SOCKADDR_IN& stRemote, const SOCKADDR_IN& stDstRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	const PU_PACKET_RELAYHEAD* pstPuRelayHd = reinterpret_cast<const PU_PACKET_RELAYHEAD*>(pbyBuff);

	if (PU_PKT_HEAD_TYPE_REQSYN == pstPuRelayHd->m_byDstType)
	{
		PU_PACKET_SYNBODY* pstPuSynBd = const_cast<PU_PACKET_SYNBODY*>(reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuRelayHd + 1));

		B_LONGTOS(stRemote.sin_addr.S_un.S_addr, pstPuSynBd->m_abySrcIpE);
		B_INTTOS(stRemote.sin_port, pstPuSynBd->m_abySrcPortE);
	}
	else
	if (PU_PKT_HEAD_TYPE_REQFIN == pstPuRelayHd->m_byDstType)
	{
		PU_PACKET_FINBODY* pstPuFinBd = const_cast<PU_PACKET_FINBODY*>(reinterpret_cast<const PU_PACKET_FINBODY*>(pstPuRelayHd + 1));

		B_LONGTOS(stRemote.sin_addr.S_un.S_addr, pstPuFinBd->m_abySrcIpE);
		B_INTTOS(stRemote.sin_port, pstPuFinBd->m_abySrcPortE);
	}

//	m_pcParent->OnRelayDataPeer(stRemote, stDstRemote, enumAddrType, pstPuRelayHd->m_byDstType, ulLen, pbyBuff);
}

void
CPuPeers::OnNewPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
 	const PU_PACKET_HEAD* pstPuHd = reinterpret_cast<const PU_PACKET_HEAD*>(pbyBuff);
 	const PU_PACKET_NEWPEERBODY* pstPuBd = reinterpret_cast<const PU_PACKET_NEWPEERBODY*>(pstPuHd + 1);
// 
// 	//
// 	SOCKADDR_IN stNewRemoteL;
// 	SOCKADDR_IN stNewRemoteE;
// 
// 	stNewRemoteL.sin_addr.S_un.S_addr = B_STOLONG(pstPuBd->m_abyNewIpL);
// 	stNewRemoteL.sin_port = B_STOINT(pstPuBd->m_abyNewPortL);
// 
// 	stNewRemoteE.sin_addr.S_un.S_addr = B_STOLONG(pstPuBd->m_abyNewIpE);
// 	stNewRemoteE.sin_port = B_STOINT(pstPuBd->m_abyNewPortE);
// 
// 	//
// 	BYTE byIndex;
// 
// 	//
// 	if (TRUE == AddPeer(
// 					B_STOLONG(pstPuBd->m_abyNewIpL),
// 					B_STOINT(pstPuBd->m_abyNewPortL),
// 					B_STOLONG(pstPuBd->m_abyNewIpE),
// 					B_STOINT(pstPuBd->m_abyNewPortE),
// 					B_STOLONG(pstPuBd->m_abyNewIpE),
// 					B_STOINT(pstPuBd->m_abyNewPortE),
// 					byIndex))
// 	{
// 		// 새로 추가
// 		CAMBUFFER tempWebcam;
// 		VOICEBUFFER tempVoice;
// 
// 		tempVoice.ulIdx = tempWebcam.ulIdx = B_STOLONG(pstPuBd->m_abyNewIpE);
// 
// 		_vecWebCamBuf.push_back( tempWebcam );
// 		tempVoice.bConnect = true;
// 		_vecVoiceBuf.push_back( tempVoice );
// 	}
// 	else
// 	{
// 		// 기존에 있는 피어 추가
// 		CAMBUFFER tempWebcam;
// 		VOICEBUFFER tempVoice;
// 
// 		tempVoice.ulIdx = tempWebcam.ulIdx = B_STOLONG(pstPuBd->m_abyNewIpE);
// 
// 		ULONG ulCheck = 0;
// 		for( int i = 0; i < _vecWebCamBuf.size(); i++ )
// 		{
// 			if( tempWebcam.ulIdx == _vecWebCamBuf[i].ulIdx )
// 			{
// 				ulCheck = tempWebcam.ulIdx;
// 				break;
// 			}
// 		}
// 		if( !ulCheck )
// 			_vecWebCamBuf.push_back( tempWebcam );
// 
// 		for( int i = 0; i < _vecVoiceBuf.size(); i++ )
// 		{
// 			if( tempVoice.ulIdx == _vecVoiceBuf[i].ulIdx )
// 			{
// 				ulCheck = tempVoice.ulIdx;
// 				break;
// 			}
// 		}
// 		if( !ulCheck )
// 			_vecVoiceBuf.push_back( tempVoice );
// 
// 		tempVoice.bConnect = true;
// 
// 	}
// 
// 	int a = _vecVoiceBuf.size();
// 
// 	int b = 0;
// 	b =3;

//	m_pcParent->OnNewPeer(stRemote, enumAddrType, ulLen, pbyBuff);
}

void
CPuPeers::OnReqCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType)
{
//	CString cstrMsg;

//	cstrMsg.Format("From [%s, %d], REQ-CHECK-EXTERNAL-IP\n", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port));

//	m_pcParent->AddStatus(LPCSTR(cstrMsg));

	//_editIPMyExtend

	int a =0 ;
}

void
CPuPeers::OnResCheckExternalIp(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulRemoteE, USHORT uiRemotePortE)
{

	struct in_addr stRemoteE;
	stRemoteE.S_un.S_addr = ulRemoteE;

	char cstrMsg[500];
	char cstrRemote1[500];
	char cstrRemote2[500];

	CData::GetInstance()._stRemoteE.sin_addr.S_un.S_addr = ulRemoteE;
	CData::GetInstance()._stRemoteE.sin_port = uiRemotePortE;

	sprintf( cstrRemote1, "%s, %d", inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port));
	sprintf( cstrRemote2, "%s, %d", inet_ntoa(stRemoteE), ntohs(uiRemotePortE));
	sprintf( cstrMsg, "From [%s], RES-CHECK-EXTERNAL-IP [%s]\n", LPCSTR(cstrRemote1), LPCSTR(cstrRemote2));

//	m_pcParent->AddStatus(LPCSTR(cstrMsg));
}

void
CPuPeers::HandleIdle()
{

	CCtrlPeers::HandleIdle();

	CCtrlPeers::TryCheckPeers(5000);

}

void
CPuPeers::HandleError(OCFCLIENT_ERROR_CATEGORY enumErrorCategory, LONG ErrorCode, unsigned char chServiceType, ULONG ulLen, const BYTE* pbyBuff)
{
//	CString cstrMsg;
//	cstrMsg.Format("ERROR:: CATEGORY[%d], CODE[%d]\n", enumErrorCategory, ErrorCode);
//	m_pcParent->AddStatus(cstrMsg);
}

void
CPuPeers::OnQueueDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
//	m_pcParent->OnDataPeer(stRemote, enumAddrType, ulLen, pbyBuff);
}

BOOL
CPuPeers::CheckReqConnect(ULONG ulLocalE, USHORT uiLocalPortE)
{

	if (FALSE == CCtrlPeers::CheckReqConnect(ulLocalE, uiLocalPortE))
	{
		//struct in_addr stLocalE;
		//stLocalE.S_un.S_addr = ulLocalE;
	
		//SOCKADDR_IN stSelfExternal;
		//CCtrlPeers::GetSelfExternalInfo(stSelfExternal);

		//CString cstrRemote1;
		//CString cstrRemote2;
		//CString cstrMsg;

		//cstrRemote1.Format("%s, %d", inet_ntoa(stLocalE), ntohs(uiLocalPortE));
		//cstrRemote2.Format("%s, %d", inet_ntoa(stSelfExternal.sin_addr), ntohs(stSelfExternal.sin_port));
		//cstrMsg.Format("Invalid Connect Request From [%s], [%s]\n", LPCSTR(cstrRemote1), LPCSTR(cstrRemote2));

//		m_pcParent->AddStatus(LPCSTR(cstrMsg));

		return FALSE;
	}

	return TRUE;
}

void
CPuPeers::OnDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{

	ProcessDataPeer( stRemote, enumAddrType, ulLen, pbyBuff);

	return;
}
// 
// void
// CPuPeers::P2PSendVideo( int iSize, BYTE *pBuf )
// {
// 
// 	_bySendBuf[0] = PACKET_VIDEO;
// 	memcpy( _bySendBuf + 1, pBuf, iSize );
// 	TryDataForPeers( iSize+1, _bySendBuf );
// 
// }
// 
// void
// CPuPeers::P2PSendVoice( int iSize, BYTE *pBuf )
// {
// 
// 	_bySendBuf[0] = PACKET_VOICE;
// 	memcpy( _bySendBuf + 1, pBuf, iSize );
// 	TryDataForPeers( iSize+1, _bySendBuf );
// 
// }
