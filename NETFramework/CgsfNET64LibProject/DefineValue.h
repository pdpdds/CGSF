#pragma once

#include "BasePacket.h"
#include "SFConstant.h"

using namespace System;

namespace CgsfNET64Lib { 
	
	public enum class SFPACKET_TYPE : short
	{ 
		NONE = SFPACKET_NONE,
		CONNECT = SFPACKET_CONNECT,
		DATA = SFPACKET_DATA,
		RPC = SFPACKET_RPC,
		TIMER = SFPACKET_TIMER,
		SHOUTER = SFPACKET_SHOUTER,
		DISCONNECT = SFPACKET_DISCONNECT,
		DB = SFPACKET_DB,
		SERVERSHUTDOWN = SFPACKET_SERVERSHUTDOWN,
	};
	
	public enum class PACKET_OPTION_TYPE : short
	{
		NONE = 0,
		DATACRC = PACKET_OPTION_DATACRC,
		COMPRESS = PACKET_OPTION_COMPRESS,
		ENCRYPTION = PACKET_OPTION_ENCRYPTION,
		CRC_DEFAULT = PACKET_OPTION_CRC_DEFAULT,
		CRC_MD5 = PACKET_OPTION_CRC_MD5,
	};

	public enum class PACKET_PROTOCOL_TYPE : short
	{
		JSON = PACKET_PROTOCOL_JSON, 
		CGSF = PACKET_PROTOCOL_CGSF,
		PROTO_BUF = PACKET_PROTOCOL_PROTO_BUF,
		MSG_PACK = PACKET_PROTOCOL_MSG_PACK,
		AVRO = PACKET_PROTOCOL_AVRO,
	};

	public ref struct NetworkConfig
	{
		String^ IP;
		UINT16 Port;
		String^ EngineDllName;
		int MaxAcceptCount;

		int ThreadCount;
		int ProtocolOption;
		int ProtocolID;
		int MaxBufferSize;
		int MaxPacketSize;
	};


	public ref struct RemoteServerConnectInfo
	{
		String^ IP;
		UINT16 Port;
		int ConnectID;
		String^ Description;

		int ProtocolOption;
		int ProtocolID;
		int MaxBufferSize;
		int MaxPacketSize;
	};

	public ref struct MultiListenNetworkInfo
	{
		int ListenID;		// 시스템에서 할당한다.
		UINT16 Port;
		
		int ProtocolOption;
		int ProtocolID;
		int MaxBufferSize;
		int MaxPacketSize;
	};

	const int CGSF_PACKET_OPTION_NONE = 0;

	const int MIN_SERVER_CONNECTOR_ID = 1001;
}