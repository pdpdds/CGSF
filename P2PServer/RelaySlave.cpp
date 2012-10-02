////////////////////////////////////////////////////////////////////////////////
// 서브시스템  :
// 프로그램 ID : RelaySlave.cc
// 기능요약    :
// 작성일      : 2007.11.20
// 작성자      : 이용휴
// 수정일	   : 2009.07.15
// 수정자      : 신상우
// 수정내역    : 리눅스용에서 윈도우용으로 수정
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
#include "AllRelay.h"

////////////////////////////////////////////////////////////////////////////////
//
CRelaySlave::CRelaySlave()
{
	//
	memset(m_pfFunc, 0x00, sizeof(m_pfFunc));

	//
	m_pfFunc[PU_PKT_HEAD_TYPE_REQECHO] = &CRelaySlave::PktHandleReqEcho;
	m_pfFunc[PU_PKT_HEAD_TYPE_RELAYDATA] = &CRelaySlave::PktHandleRelayData;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQCHECKEXTERNALIP] = &CRelaySlave::PktHandleReqCheckExternalIp;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQSYN] = &CRelaySlave::PktHandleReqSyn;
	m_pfFunc[PU_PKT_HEAD_TYPE_REQFIN] = &CRelaySlave::PktHandleReqFin;

	m_pfFunc[PU_PKT_HEAD_TYPE_RELAYDATA_EX] = &CRelaySlave::PktHandleRelayDataEx;


}

CRelaySlave::~CRelaySlave()
{
}

// BOOL
// CRelaySlave::Start()
// {
// 	return TRUE;
// }

BOOL
CRelaySlave::Do()
{
	int nRet;
	int nLen;
	struct sockaddr_in stRemote;
	PU_PACKET_HEAD* pstPuHd;

	//
	while (TRUE)
	{
		nLen = sizeof(stRemote);

		//
	    nRet = recvfrom(
					m_dSock,
					reinterpret_cast<char*>(m_abyRcvData),
					sizeof(m_abyRcvData),
					0,
					reinterpret_cast<struct sockaddr *>(&stRemote),
					&nLen);
		if (0 > nRet)
		{
			continue;
		}

		if (nRet < PU_PACKET_HEAD_LEN)
		{
			continue;
		}

		//
		pstPuHd = reinterpret_cast<PU_PACKET_HEAD*>(m_abyRcvData);

		//
		if (NULL != m_pfFunc[pstPuHd->m_byType])
		{
			(this->*m_pfFunc[pstPuHd->m_byType])(stRemote, pstPuHd, nRet, m_abyRcvData);
		}
	}
}

// BOOL
// CRelaySlave::ThreadEnd()
// {
// 	return TRUE;
// }

BOOL
CRelaySlave::SendResEcho(
				const sockaddr_in& stRemote,
				BYTE byAddrType,
				ULONG ulSeq,
				ULONG ulSerial,
				UINT uiLen,
				const BYTE* pbyBuff)
{
	int nRet;

	//
	int nLenPkt = sizeof(PU_PACKET_HEAD) + uiLen;

	//
	PU_PACKET_HEAD* pstHd = reinterpret_cast<PU_PACKET_HEAD*>(m_abySndData);
	BYTE* pbyBd = reinterpret_cast<BYTE*>(pstHd + 1);	

	//
	if (nLenPkt >= RELAY_PKT_MAXSIZE)
	{
		return FALSE;
	}

	//
	pstHd->m_byType = PU_PKT_HEAD_TYPE_RESECHO;
	pstHd->m_byAddrType = byAddrType;
	B_INTTOS(uiLen, pstHd->m_abyLen);
	B_LONGTOS(ulSeq, pstHd->m_abySeqAck);
	B_LONGTOS(ulSerial, pstHd->m_abySerial);

	//
	memcpy(pbyBd, pbyBuff, uiLen);

	//
	nRet = sendto(m_dSock, reinterpret_cast<char*>(m_abySndData), nLenPkt, 0, reinterpret_cast<const struct sockaddr*>(&stRemote), sizeof(stRemote));
	if (0 > nRet)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
CRelaySlave::SendResCheckExternalIp(
				const sockaddr_in& stRemote,
				BYTE byAddrType,
				ULONG ulSeq,
				ULONG ulSerial)
{
	int nRet;

	//
	int nLenPkt = sizeof(PU_PACKET_HEAD) + sizeof(PU_PACKET_CHECKEXTERNALIP);

	//
	PU_PACKET_HEAD* pstHd = reinterpret_cast<PU_PACKET_HEAD*>(m_abySndData);
	PU_PACKET_CHECKEXTERNALIP* pstCheckExternalIp = reinterpret_cast<PU_PACKET_CHECKEXTERNALIP*>(pstHd + 1);

	//
	if (nLenPkt >= RELAY_PKT_MAXSIZE)
	{
		return FALSE;
	}

	//
	pstHd->m_byType = PU_PKT_HEAD_TYPE_RESCHECKEXTERNALIP;
	pstHd->m_byAddrType = byAddrType;
	B_INTTOS(sizeof(PU_PACKET_CHECKEXTERNALIP), pstHd->m_abyLen);
	B_LONGTOS(ulSeq, pstHd->m_abySeqAck);
	B_LONGTOS(ulSerial, pstHd->m_abySerial);

	//
	B_LONGTOS(stRemote.sin_addr.s_addr, pstCheckExternalIp->m_abyIpE);
	B_INTTOS(stRemote.sin_port, pstCheckExternalIp->m_abyPortE);

	//
	nRet = sendto(m_dSock, reinterpret_cast<char*>(m_abySndData), nLenPkt, 0, reinterpret_cast<const struct sockaddr*>(&stRemote), sizeof(stRemote));
	if (0 > nRet)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
CRelaySlave::SendResSyn(
			const sockaddr_in& stRemote,
			BYTE byAddrType,
			ULONG ulSeq,
			ULONG ulSerial,
			UINT uiLen,
			const BYTE* pbyBuff)
{
	int nRet;

	//
	int nLenPkt = sizeof(PU_PACKET_HEAD) + uiLen;

	//
	PU_PACKET_HEAD* pstHd = reinterpret_cast<PU_PACKET_HEAD*>(m_abySndData);
	BYTE* pbyBd = reinterpret_cast<BYTE*>(pstHd + 1);	

	//
	if (nLenPkt >= RELAY_PKT_MAXSIZE)
	{
		return FALSE;
	}

	//
	pstHd->m_byType = PU_PKT_HEAD_TYPE_RESSYN;
	pstHd->m_byAddrType = byAddrType;
	B_INTTOS(uiLen, pstHd->m_abyLen);
	B_LONGTOS(ulSeq, pstHd->m_abySeqAck);
	B_LONGTOS(ulSerial, pstHd->m_abySerial);

	//
	memcpy(pbyBd, pbyBuff, uiLen);

	//
	nRet = sendto(m_dSock, reinterpret_cast<char*>(m_abySndData), nLenPkt, 0, reinterpret_cast<const struct sockaddr*>(&stRemote), sizeof(stRemote));
	if (0 > nRet)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
CRelaySlave::SendResFin(
			const sockaddr_in& stRemote,
			BYTE byAddrType,
			ULONG ulSeq,
			ULONG ulSerial,
			UINT uiLen,
			const BYTE* pbyBuff)
{
	int nRet;

	//
	int nLenPkt = sizeof(PU_PACKET_HEAD) + uiLen;

	//
	PU_PACKET_HEAD* pstHd = reinterpret_cast<PU_PACKET_HEAD*>(m_abySndData);
	BYTE* pbyBd = reinterpret_cast<BYTE*>(pstHd + 1);	

	//
	if (nLenPkt >= RELAY_PKT_MAXSIZE)
	{
		return FALSE;
	}

	//
	pstHd->m_byType = PU_PKT_HEAD_TYPE_RESFIN;
	pstHd->m_byAddrType = byAddrType;
	B_INTTOS(uiLen, pstHd->m_abyLen);
	B_LONGTOS(ulSeq, pstHd->m_abySeqAck);
	B_LONGTOS(ulSerial, pstHd->m_abySerial);

	//
	memcpy(pbyBd, pbyBuff, uiLen);

	//
	nRet = sendto(m_dSock, reinterpret_cast<char*>(m_abySndData), nLenPkt, 0, reinterpret_cast<const struct sockaddr*>(&stRemote), sizeof(stRemote));
	if (0 > nRet)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
CRelaySlave::Send(
			const sockaddr_in& stRemote,
			BYTE byType,
			BYTE byAddrType,
			ULONG ulSeq,
			ULONG ulSerial,
			UINT uiLen,
			const BYTE* pbyBuff)
{
	int nRet;

	//
	int nLenPkt = sizeof(PU_PACKET_HEAD) + uiLen;

	//
	PU_PACKET_HEAD* pstHd = reinterpret_cast<PU_PACKET_HEAD*>(m_abySndData);
	BYTE* pbyBd = reinterpret_cast<BYTE*>(pstHd + 1);	

	//
	if (nLenPkt >= RELAY_PKT_MAXSIZE)
	{
		return FALSE;
	}

	//
	pstHd->m_byType = byType;
	pstHd->m_byAddrType = byAddrType;
	B_INTTOS(uiLen, pstHd->m_abyLen);
	B_LONGTOS(ulSeq, pstHd->m_abySeqAck);
	B_LONGTOS(ulSerial, pstHd->m_abySerial);

	//
	memcpy(pbyBd, pbyBuff, uiLen);

	//
	nRet = sendto(m_dSock, reinterpret_cast<char*>(m_abySndData), nLenPkt, 0, reinterpret_cast<const struct sockaddr*>(&stRemote), sizeof(stRemote));
	if (0 > nRet)
	{
		return FALSE;
	}

	return TRUE;
}

void
CRelaySlave::PktHandleReqEcho(
			const sockaddr_in& stRemote,
			const PU_PACKET_HEAD* pstPuHd,
			ULONG ulLen,
			const BYTE* pbyBuff)
{
	SendResEcho(stRemote,
				pstPuHd->m_byAddrType,
				B_STOLONG(pstPuHd->m_abySeqAck),
				B_STOLONG(pstPuHd->m_abySerial),
				static_cast<UINT>(ulLen - PU_PACKET_HEAD_LEN),
				reinterpret_cast<const BYTE*>(pstPuHd + 1));
}

void
CRelaySlave::PktHandleRelayData(
			const sockaddr_in& stRemote,
			const PU_PACKET_HEAD* pstPuHd,
			ULONG ulLen,
			const BYTE* pbyBuff)
{
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
	struct sockaddr_in stDstRemote;

	stDstRemote.sin_family = AF_INET;
	stDstRemote.sin_addr.s_addr = B_STOLONG(pstPuRelayHd->m_abyDstIP);
	stDstRemote.sin_port = B_STOINT(pstPuRelayHd->m_abyDstPort);

	//
	Send(
		stDstRemote,
		pstPuRelayHd->m_byDstType,
		pstPuRelayHd->m_byDstAddrType,
		B_STOLONG(pstPuRelayHd->m_abySeqAck),
		B_STOLONG(pstPuRelayHd->m_abySerial),
		B_STOINT(pstPuRelayHd->m_abyLen),
		reinterpret_cast<const BYTE*>(pstPuRelayHd + 1));
}

void
CRelaySlave::PktHandleRelayDataEx(
								const sockaddr_in& stRemote,
								const PU_PACKET_HEAD* pstPuHd,
								ULONG ulLen,
								const BYTE* pbyBuff)
{
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

	//
	ULONG ulRelayDestHeadSize = PU_PACKET_RELAYDESTHEAD_EX_LEN;
	ULONG ulHeadSize = PU_PACKET_RELAYHEAD_EX_LEN;
	ULONG ulDestArraySize = static_cast<BYTE>(pstPuRelayHd->m_byDstSize);

	//
	PU_PACKET_RELAYDESTHEAD_EX stDestHead[256] = {0,};

	//
	for(ULONG uli = 0; uli < ulDestArraySize ; uli++)
	{
		memcpy( &stDestHead[uli] ,  &pbyBuff[ulHeadSize + (ulRelayDestHeadSize * uli) ] , ulRelayDestHeadSize );
	}

	//
	ULONG ulStartRelayPktSize = (ulRelayDestHeadSize * ulDestArraySize) + ulHeadSize;
	ULONG ulRelayPktSize = ulLen - ulStartRelayPktSize;

	//
	for(ULONG uli = 0 ; uli < ulDestArraySize ; uli++)
	{

		//
		struct sockaddr_in stDstRemote = { 0, };

		stDstRemote.sin_family = AF_INET;
		stDstRemote.sin_addr.s_addr = B_STOLONG(stDestHead[uli].m_abyDstIP);
		stDstRemote.sin_port = B_STOINT(stDestHead[uli].m_abyDstPort);

		//
		Send(
			stDstRemote,
			pstPuRelayHd->m_byDstType,
			pstPuRelayHd->m_byDstAddrType,
			B_STOLONG(pstPuRelayHd->m_abySeqAck),
			B_STOLONG(pstPuRelayHd->m_abySerial),
			B_STOINT(pstPuRelayHd->m_abyLen),
			reinterpret_cast<const BYTE*>(pbyBuff + ulStartRelayPktSize));
	}


}

void
CRelaySlave::PktHandleReqCheckExternalIp(
			const sockaddr_in& stRemote,
			const PU_PACKET_HEAD* pstPuHd,
			ULONG,
			const BYTE*)
{
	SendResCheckExternalIp(
		stRemote,
		pstPuHd->m_byAddrType,
		B_STOLONG(pstPuHd->m_abySeqAck),
		B_STOLONG(pstPuHd->m_abySerial));
}

void
CRelaySlave::PktHandleReqSyn(const struct sockaddr_in& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (PU_PACKET_SYNBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	const PU_PACKET_SYNBODY* pstSynBd = reinterpret_cast<const PU_PACKET_SYNBODY*>(pstPuHd + 1);
	
	//
	if (PU_PEERADDRTYPE_RELAY != pstPuHd->m_byAddrType)
	{
		SendResSyn(
				stRemote,
				pstPuHd->m_byAddrType,
				B_STOLONG(pstPuHd->m_abySeqAck),
				B_STOLONG(pstPuHd->m_abySerial),
				static_cast<UINT>(ulLen - PU_PACKET_HEAD_LEN),
				reinterpret_cast<const BYTE*>(pstPuHd + 1));
	}
}

void
CRelaySlave::PktHandleReqFin(const struct sockaddr_in& stRemote, const PU_PACKET_HEAD* pstPuHd, ULONG ulLen, const BYTE* pbyBuff)
{
	if (PU_PACKET_FINBODY_LEN != B_STOINT(pstPuHd->m_abyLen))
	{
		return;
	}

	const PU_PACKET_FINBODY* pstFinBd = reinterpret_cast<const PU_PACKET_FINBODY*>(pstPuHd + 1);
	
	//
	if (PU_PEERADDRTYPE_RELAY != pstPuHd->m_byAddrType)
	{
		SendResFin(
				stRemote,
				static_cast<PU_PEERADDRTYPE>(pstPuHd->m_byAddrType),
				B_STOLONG(pstPuHd->m_abySeqAck),
				B_STOLONG(pstPuHd->m_abySerial),
				static_cast<UINT>(ulLen - PU_PACKET_HEAD_LEN),
				reinterpret_cast<const BYTE*>(pstPuHd + 1));
	}
}

