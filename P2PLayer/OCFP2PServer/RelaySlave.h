////////////////////////////////////////////////////////////////////////////////
// 서브시스템  :
// 프로그램 ID : RelaySlave.h
// 기능요약    :
// 작성일      : 2007.11.20
// 작성자      : 이용휴
// 수정일	   : 2009.07.15
// 수정자      : 신상우
// 수정내역    : 리눅스용에서 윈도우용으로 수정
////////////////////////////////////////////////////////////////////////////////

#ifndef __RELAYSLAVE_H__
#define __RELAYSLAVE_H__

////////////////////////////////////////////////////////////////////////////////
//
#define RELAY_PKT_MAXSIZE					0xFFFF

////////////////////////////////////////////////////////////////////////////////
//
class CRelaySlave: public CThreadClient
{
// START
typedef
	void (CRelaySlave::*RELAYPKTFUNC)(
			const sockaddr_in&,
			const PU_PACKET_HEAD*,
			ULONG,
			const BYTE*);
// STOP

public:
	CRelaySlave();
	virtual ~CRelaySlave();

public:
	inline void SetSock(SOCKET dSock) { m_dSock = dSock; }

//     virtual BOOL Start();
     virtual BOOL Do();
//     virtual BOOL ThreadEnd();

private:
	BOOL SendResEcho(const sockaddr_in&, BYTE, ULONG, ULONG, UINT, const BYTE*);
	BOOL SendResCheckExternalIp(const sockaddr_in&, BYTE, ULONG, ULONG);
	BOOL Send(const sockaddr_in&, BYTE, BYTE, ULONG, ULONG, UINT, const BYTE*);
	BOOL SendResSyn(const sockaddr_in&, BYTE, ULONG, ULONG, UINT, const BYTE*);
	BOOL SendResFin(const sockaddr_in&, BYTE, ULONG, ULONG, UINT, const BYTE*);

private:
	void PktHandleReqEcho(const sockaddr_in&, const PU_PACKET_HEAD*, ULONG, const BYTE*);
	void PktHandleRelayData(const sockaddr_in&, const PU_PACKET_HEAD*, ULONG, const BYTE*);
	void PktHandleReqCheckExternalIp(const sockaddr_in&, const PU_PACKET_HEAD*, ULONG, const BYTE*);
	void PktHandleReqSyn(const sockaddr_in&, const PU_PACKET_HEAD*, ULONG, const BYTE*);
	void PktHandleReqFin(const sockaddr_in&, const PU_PACKET_HEAD*, ULONG, const BYTE*);

	void PktHandleRelayDataEx(const sockaddr_in&, const PU_PACKET_HEAD*, ULONG, const BYTE*);

private:
	SOCKET m_dSock;
	BYTE m_abySndData[RELAY_PKT_MAXSIZE];
	BYTE m_abyRcvData[RELAY_PKT_MAXSIZE];

private:
	RELAYPKTFUNC m_pfFunc[PU_PKT_HEAD_TYPE_END];
};
//

#endif

