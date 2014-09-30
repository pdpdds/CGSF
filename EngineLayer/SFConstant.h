#pragma once

#define MAX_IO_SIZE 16384 //시스템상에서 제한을 두는 IO 최대치
#define MAX_PACKET_SIZE 8192 //시스템상에서 제한을 두는 패킷 사이즈 최대치

#define ENCRYPTION_KEY 0xAFB7E3D9

#define PACKET_DEFAULT_IO_SIZE					8192 //디폴트 IO 사이즈
#define PACKET_DEFAULT_PACKET_SIZE				4096 //디폴트 패킷 사이즈

#define PACKETIO_ERROR_NONE				0
#define PACKETIO_ERROR_HEADER			-1
#define PACKETIO_ERROR_DATA				-2

#define PACKETIO_ERROR_DATA_CRC			-3
#define PACKETIO_ERROR_DATA_COMPRESS	-4
#define PACKETIO_ERROR_DATA_ENCRYPTION	-5

#define PACKET_COMPRESS_LIMIT 1024

/////////////////////////////////////////////////////////////////////////
#define PACKET_OPTION_DATACRC			0x00000001		// 지원언어: C++
#define PACKET_OPTION_COMPRESS			0x00000002		// 지원언어: C++, C#
#define PACKET_OPTION_ENCRYPTION		0x00000004		// 지원언어: C++
#define PACKET_OPTION_CRC_DEFAULT		0x00000008		// 지원언어: C++
#define PACKET_OPTION_CRC_MD5			0x00000010		// 지원언어: C++
#define PACKET_OPTION_ENCRYPTION_DES	0x00000020		// 지원언어: C#
#define PACKET_OPTION_ENCRYPTION_3DES	0x00000040		// 지원언어: C#
#define PACKET_OPTION_ENCRYPTION_AES	0x00000080		// 지원언어: C#

#define PACKET_PROTOCOL_JSON		1
#define PACKET_PROTOCOL_CGSF		2
#define PACKET_PROTOCOL_PROTO_BUF	3
#define PACKET_PROTOCOL_MSG_PACK	4
#define PACKET_PROTOCOL_AVRO		5

/////////////////////////////////////////////////////////////////////////
#define TIMER_100_MSEC	 1
#define TIMER_200_MSEC	 2
#define TIMER_500_MSEC	 3
#define TIMER_1_SEC		 4
#define TIMER_2_SEC		 5
#define TIMER_5_SEC		 6
#define TIMER_10_SEC	 7

/////////////////////////////////////////////////////////////////////////
typedef enum eDBResult
{
	DBRESULT_ERROR_UNKNOWN = -999,
	DBRESULT_STATE_SUCCESS = 0,
};

////////////////////////////////////////////////////////////////////////
//MACRO
////////////////////////////////////////////////////////////////////////
#define SF_GETPACKET_ARG(a,b,c) memcpy(a,b.c_str(), sizeof(c));
#define SF_SAFE_RELEASE(a) if(a) delete a;

#define SFPACKET_RPC_PACKET 60000
