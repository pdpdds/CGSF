#pragma once

#include "BasePacket.h"
#include "SFConstant.h"

using namespace System;

namespace CgsfNET64Lib { 
	
	/// <summary>
	/// 패킷 타입
	/// </summary>
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
	
	/// <summary>
	/// 패킷 옵션
	/// </summary>
	public enum class PACKET_OPTION_TYPE : short
	{
		NONE = 0,
		DATACRC = PACKET_OPTION_DATACRC,
		COMPRESS = PACKET_OPTION_COMPRESS,
		ENCRYPTION = PACKET_OPTION_ENCRYPTION,
		CRC_DEFAULT = PACKET_OPTION_CRC_DEFAULT,
		CRC_MD5 = PACKET_OPTION_CRC_MD5,
		ENCRYPTION_DES = PACKET_OPTION_ENCRYPTION_DES,
		ENCRYPTION_3DES = PACKET_OPTION_ENCRYPTION_3DES,
		ENCRYPTION_AES = PACKET_OPTION_ENCRYPTION_AES,
	};

	/// <summary>
	/// 프로토콜 타입
	/// </summary>
	public enum class PACKET_PROTOCOL_TYPE : short
	{
		JSON = PACKET_PROTOCOL_JSON, 
		CGSF = PACKET_PROTOCOL_CGSF,
		PROTO_BUF = PACKET_PROTOCOL_PROTO_BUF,
		MSG_PACK = PACKET_PROTOCOL_MSG_PACK,
		AVRO = PACKET_PROTOCOL_AVRO,
	};



	/// <summary>
	/// 네트워크 설정
	/// </summary>
	public ref struct NetworkConfig
	{
		String^ IP;
		UINT16 Port;
		
		/// 네트워크 엔진 dll 파일 이름
		String^ EngineDllName;
		
		/// 패킷 디스패쳐 스레드 수
		int ThreadCount;

		/// 최대 접속 허용 수
		int MaxAcceptCount;
		int ProtocolOption;
		int ProtocolID;
		int MaxBufferSize;
		int MaxPacketSize;
	};

	/// <summary>
	/// 다른 서버 접속을 위한 네트워크 정보
	/// </summary>
	public ref struct RemoteServerConnectInfo
	{
		String^ IP;
		UINT16 Port;
		/// 연결 ID. 중복되면 안된다
		int ConnectID;
		String^ Description;

		int ProtocolOption;
		int ProtocolID;
		int MaxBufferSize;
		int MaxPacketSize;
	};

	/// <summary>
	/// 멀티 listen 네트워크 정보
	/// </summary>
	public ref struct MultiListenNetworkInfo
	{
		/// listen ID. 시스템에서 할당한다.
		int ListenID;		
				
		UINT16 Port;
		
		int ProtocolOption;
		int ProtocolID;
		int MaxBufferSize;
		int MaxPacketSize;
	};

	/// 패킷 옵션 선언
	const int CGSF_PACKET_OPTION_NONE = 0;

	/// 서버 컨넥터 ID의 최소 수. 이것보다 작으면 안된다.
	const int MIN_SERVER_CONNECTOR_ID = 1001;
}