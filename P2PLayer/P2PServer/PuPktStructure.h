////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 
// 설  명 :
// 수정일	   : 2009.07.15
// 수정자      : 신상우
// 수정내역    : 리눅스용에서 윈도우용으로 수정
//

#ifndef __PUPKTSTRUCTURE_H__
#define __PUPKTSTRUCTURE_H__

////////////////////////////////////////////////////////////////////////////////
//
typedef enum
{
	PU_PEERADDRTYPE_NONE = 0,
	PU_PEERADDRTYPE_LOCAL = 1,
	PU_PEERADDRTYPE_EXTERNAL = 2,
	PU_PEERADDRTYPE_EXTERNAL2 = 3,
	PU_PEERADDRTYPE_RELAY = 4
} PU_PEERADDRTYPE;

////////////////////////////////////////////////////////////////////////////////
//
#define PU_MAXDOTADDRESS_LEN			256

////////////////////////////////////////////////////////////////////////////////
//
#define PU_PKT_HEAD_SEQ_LEN				4
#define PU_PKT_HEAD_LEN_LEN				2
#define PU_PKT_HEAD_IP_LEN				4
#define PU_PKT_HEAD_PORT_LEN			2
#define PU_PKT_HEAD_SERIAL_LEN			4

////////////////////////////////////////////////////////////////////////////////
//
typedef enum 
{
	PU_PKT_HEAD_TYPE_START = 0,			// 송수신에 사용하지 않음..
	PU_PKT_HEAD_TYPE_REQECHO = 1,
	PU_PKT_HEAD_TYPE_RESECHO = 2,
	PU_PKT_HEAD_TYPE_REQSYN = 3,
	PU_PKT_HEAD_TYPE_RESSYN = 4,
	PU_PKT_HEAD_TYPE_REQFIN = 5,
	PU_PKT_HEAD_TYPE_RESFIN = 6,
	PU_PKT_HEAD_TYPE_RESET = 7,
	PU_PKT_HEAD_TYPE_DATA = 8,
	PU_PKT_HEAD_TYPE_RELAYDATA = 9,
	PU_PKT_HEAD_TYPE_NEWPEER = 10,
	PU_PKT_HEAD_TYPE_REQCHECKEXTERNALIP = 11,
	PU_PKT_HEAD_TYPE_RESCHECKEXTERNALIP = 12,
	
	PU_PKT_HEAD_TYPE_RELAYDATA_EX = 15,

	PU_PKT_HEAD_TYPE_END = 255			// 송수신에 사용하지 않음..
} PU_PKT_HEAD_TYPE;

////////////////////////////////////////////////////////////////////////////////
//
// Packet Head
//
typedef struct
{
	BYTE m_byType;
	BYTE m_byAddrType;
	BYTE m_abyLen[PU_PKT_HEAD_LEN_LEN];
	BYTE m_abySeqAck[PU_PKT_HEAD_SEQ_LEN];
	BYTE m_abySerial[PU_PKT_HEAD_SERIAL_LEN];
} PU_PACKET_HEAD;

#define PU_PACKET_HEAD_LEN				(sizeof(PU_PACKET_HEAD))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_byType;
	BYTE m_byAddrType;
	BYTE m_abyLen[PU_PKT_HEAD_LEN_LEN];
	BYTE m_abySeqAck[PU_PKT_HEAD_SEQ_LEN];
	BYTE m_abySerial[PU_PKT_HEAD_SERIAL_LEN];
	BYTE m_byDstType;
	BYTE m_byDstAddrType;
	BYTE m_abyDstIP[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPort[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_RELAYHEAD;

#define PU_PACKET_RELAYHEAD_LEN			(sizeof(PU_PACKET_RELAYHEAD))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_byType;
	BYTE m_byAddrType;
	BYTE m_abyLen[PU_PKT_HEAD_LEN_LEN];
	BYTE m_abySeqAck[PU_PKT_HEAD_SEQ_LEN];
	BYTE m_abySerial[PU_PKT_HEAD_SERIAL_LEN];
	BYTE m_byDstType;
	BYTE m_byDstAddrType;
	BYTE m_byDstSize;

} PU_PACKET_RELAYHEAD_EX;

#define PU_PACKET_RELAYHEAD_EX_LEN			(sizeof(PU_PACKET_RELAYHEAD_EX))

typedef struct 
{
	BYTE m_abyDstIP[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPort[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_RELAYDESTHEAD_EX;

#define PU_PACKET_RELAYDESTHEAD_EX_LEN			(sizeof(PU_PACKET_RELAYDESTHEAD_EX))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_byDstIndex;
	BYTE m_abyDstIpL[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPortL[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abyDstIpE[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPortE[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abySrcIpL[PU_PKT_HEAD_IP_LEN];
	BYTE m_abySrcPortL[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abySrcIpE[PU_PKT_HEAD_IP_LEN];
	BYTE m_abySrcPortE[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_SYNBODY;

#define PU_PACKET_SYNBODY_LEN			(sizeof(PU_PACKET_SYNBODY))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_byDstIndex;
	BYTE m_abyDstIpL[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPortL[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abyDstIpE[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPortE[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abySrcIpL[PU_PKT_HEAD_IP_LEN];
	BYTE m_abySrcPortL[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abySrcIpE[PU_PKT_HEAD_IP_LEN];
	BYTE m_abySrcPortE[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_FINBODY;

#define PU_PACKET_FINBODY_LEN			(sizeof(PU_PACKET_FINBODY))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_abyNewIpL[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyNewPortL[PU_PKT_HEAD_PORT_LEN];
	BYTE m_abyNewIpE[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyNewPortE[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_NEWPEERBODY;

#define PU_PACKET_NEWPEERBODY_LEN		(sizeof(PU_PACKET_NEWPEERBODY))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_byDstIndex;
} PU_PACKET_ECHOBODY;

#define PU_PACKET_ECHOBODY_LEN			(sizeof(PU_PACKET_ECHOBODY))

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_abyIpE[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyPortE[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_CHECKEXTERNALIP;

#define PU_PACKET_CHECKEXTERNALIP_LEN	(sizeof(PU_PACKET_CHECKEXTERNALIP))

#endif

