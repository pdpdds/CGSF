////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 
// 설  명 :
//

#ifndef __PUPKTSTRUCTURE_H__
#define __PUPKTSTRUCTURE_H__

////////////////////////////////////////////////////////////////////////////////
//
#define B_STOINT(s)						((((s)[0]<<8)&0xff00)|((s)[1]&0x00ff))
#define B_STOLONG(s)					((((s)[0]<<24)&0xff000000)| \
										 (((s)[1]<<16)&0x00ff0000)| \
										 (((s)[2]<<8)&0x0000ff00)| \
										  ((s)[3]&0x000000ff))
#define B_INTTOS(i,s)					{(s)[1]=(unsigned char)(i&0xff); \
										 (s)[0]=(unsigned char)((i>>8)&0xff);}
#define B_LONGTOS(l,s)					{(s)[3]=(unsigned char)(l&0xff); \
										 (s)[2]=(unsigned char)((l>>8)&0xff); \
										 (s)[1]=(unsigned char)((l>>16)&0xff); \
										 (s)[0]=(unsigned char)((l>>24)&0xff);}


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
	PU_PKT_HEAD_TYPE_REQCHECKRELAYABLEPEER = 13,			// 방화벽 찾기
	PU_PKT_HEAD_TYPE_RESCHECKRELAYABLEPEER = 14,

	PU_PKT_HEAD_TYPE_RELAYDATA_EX = 15,	
	
	PU_PKT_HEAD_TYPE_CHKSPEED = 16,



	PU_PKT_HEAD_TYPE_END = 255			// 송수신에 사용하지 않음..
} PU_PKT_HEAD_TYPE;

////////////////////////////////////////////////////////
//
typedef enum
{
	PU_DATA_TYPE_NONE		= 0,	//보내는 데이터 없음
	PU_DATA_TYPE_VIDEO		= 1,	//영상데이터
	PU_DATA_TYPE_AUDIO		= 2,	//음성데이터
	PU_DATA_TYPE_IMAGE		= 4,	//이미지 데이터
	PU_DATA_TYPE_ONENONE	= 8,	//1:1
	PU_DATA_TYPE_ETC		= 16 	//기타

} PU_DATA_TYPE;

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
	BYTE m_abyDstIP[PU_PKT_HEAD_IP_LEN];
	BYTE m_abyDstPort[PU_PKT_HEAD_PORT_LEN];
} PU_PACKET_RELAYDESTHEAD_EX;

#define PU_PACKET_RELAYDESTHEAD_EX_LEN			(sizeof(PU_PACKET_RELAYDESTHEAD_EX))

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

////////////////////////////////////////////////////////////////////////////////
//
typedef struct
{
	BYTE m_abySrcIP[PU_PKT_HEAD_IP_LEN];			// 보낸IP
	BYTE m_abySrcPort[PU_PKT_HEAD_PORT_LEN];		// 보낸PORT
} PU_PACKET_MYIP;

#define PU_PACKET_MYIP_LEN	(sizeof(PU_PACKET_MYIP))

typedef struct  
{

	ULONG ulRemoteL;
	USHORT uiRemotePortL;
	ULONG ulRemoteE;
	USHORT uiRemotePortE;

} PU_PEER_ADDRESS;

#endif
