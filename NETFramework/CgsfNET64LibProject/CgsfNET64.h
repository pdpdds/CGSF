// EngineLayerNET.h

#pragma once

#include "SFNETPacket.h"
#include "FRAMEWORK_ERROR_CODE.h"
#include "ConcurrencyPacketQueue.h"

using namespace System;
using namespace System::Collections;


namespace CgsfNET64Lib {

	class ServerLogicEntry;
	class SFNETDispatcher;
	class ServerConnectCallback;
	ref class ConcurrencyPacketQueue;

	public ref class CgsfNET64
	{
	public:
		CgsfNET64();
		~CgsfNET64();

		/// <summary>
		/// 네트워크 초기화
		/// </summary>
		/// <param name="config">네트워크 설정 정보</param>
		/// <param name="connectInfoList">다른 서버에 접속하기 위한 정보</param>
		/// <param name="listneInfoList">config에 설정된 Port 이외의 추가 listen 정보</param>
		/// <returns>에러코드. NET_ERROR_CODE_N::SUCCESS 이외는 실패</returns>
		NET_ERROR_CODE_N Init(NetworkConfig^ config, 
								Generic::List<RemoteServerConnectInfo^>^ connectInfoList,
								Generic::List<MultiListenNetworkInfo^>^ listneInfoList);
				

		/// <summary>
		/// 네트워크 동작 시작. 접속을 받거나 다른 서버에 접속을 할 수 있다.
		/// </summary>
		/// <param name="protocolID">네트워크 설정의 Port로 접속하는 세션에서 사용할 프로토콜ID</param>
		/// <returns></returns>
		bool Start(int protocolID);
		
		/// <summary>
		/// 네트워크 동작 중단.
		/// </summary>
		void Stop();
			
		/// <summary>
		/// remote에서 받은 패킷을 가져간다.
		/// </summary>
		/// <returns>C++ SFPacket을 닷넷으로 바꾼 패킷 데이터</returns>
		SFNETPacket^ GetPacket();

		/// <summary>
		/// 패킷 저장소에 패킷을 넣는다. 네트워크로 받은 패킷과 같은 방향으로 처리하기 원할 때 사용한다.
		/// </summary>
		/// <param name="packet">패킷</param>
		void InnerPacket(SFNETPacket^ packet);

		/// <summary>
		/// 패킷 보내기.
		/// </summary>
		/// <param name="sessionID">패킷을 보낼 세션</param>
		/// <param name="packetID">패킷 ID</param>
		/// <param name="data">보낼 데이터. 패킷의 body 부분</param>
		/// <returns>true이면 보내기 성공</returns>
		bool SendPacket(int sessionID, UINT16 packetID, array<Byte>^ data);

		/// <summary>
		/// 현재 설정된 네트워크 설정 정보를 얻는다.
		/// </summary>
		/// <returns>네트워크 설정 정보</returns>
		NetworkConfig^ GetNetworkConfig() { return m_networkConfig; }

		/// <summary>
		/// 세션을 강제적으로 짜른다.
		/// </summary>
		/// <param name="sessionID">세션</param>
		void ForceDisConnect(int sessionID);
				
		/// <summary>
		/// 리모트 서버에 접속을 시도한다. 주기적으로 접속 되지 않은 서버가 있다면 백 그라운드로 접속을 시도한다.
		/// </summary>
		/// <returns>true가 아니면 실패</returns>
		bool SetupServerReconnectSys();


		
	private:
		/// <summary>
		/// 네트워크 정보를 설정한다. C++과 다르게 설정파일을 읽지 않고 직접 설정한다.
		/// </summary>
		/// <param name="config">네트워크 설정</param>
		void SetNetworkConfig(NetworkConfig^ config);
		
		/// <summary>
		/// 패킷 프로토콜 추가
		/// </summary>
		/// <param name="protocolID">프로토콜 ID</param>
		/// <param name="maxBufferSize">최대 버퍼 크기</param>
		/// <param name="maxPacketSize">패킷 하나의 최대 크기</param>
		/// <param name="option">패킷 옵션</param>
		/// <returns>true가 아니면 추가 실패</returns>
		bool AddPacketProtocol(int protocolID, int maxBufferSize, int maxPacketSize, int option);
		
		/// <summary>
		/// 프로토콜 ID가 유니크한지 조사한다.
		/// </summary>
		/// <param name="ㅌㅌㅌㅌㅌㅌㅌ">네트워크 설정의 Port로 접속하는 세션에서 사용할 프로토콜ID</param>
		/// <returns></returns>
		bool CheckingUniqueProtocolID(int protocolID);
		
		/// <summary>
		/// 다른 서버로의 연결 정보를 등록. 이 정보를 사용하여 다른 서버에 접속을 시도한다.
		/// </summary>
		/// <param name="connectInfo">연결 정보</param>
		/// <returns>에러코드. NET_ERROR_CODE_N::SUCCESS 이외는 실패</returns>
		NET_ERROR_CODE_N RegistConnectInfo(RemoteServerConnectInfo^ connectInfo);
		
		

		/// <summary>
		/// 서버 로직 엔트리. 대부분 네트워크 관련 부분
		/// </summary>
		ServerLogicEntry* m_pLogicEntry = nullptr;
		
		/// <summary>
		/// 네트워크 엔진에서 패킷을 가져오는 클래스
		/// </summary>
		SFNETDispatcher* m_pDispatcher = nullptr;

		/// <summary>
		/// 다른 서버에 연결한 후 연결, 끊어짐 및 패킷이 오면 호출되는 콜백
		/// </summary>
		ServerConnectCallback* m_pServerConnectCallback = nullptr;
		
		/// <summary>
		/// 패킷 저장 큐. 스레드 세이프 하다
		/// </summary>
		ConcurrencyPacketQueue^ m_packetQueue = gcnew ConcurrencyPacketQueue();

		/// <summary>
		/// 네트워크 설정 정보
		/// </summary>
		NetworkConfig^ m_networkConfig = gcnew NetworkConfig();
		
		/// <summary>
		/// 다른 서버로의 접속 정보
		/// </summary>
		Generic::List<RemoteServerConnectInfo^>^ m_RemoteServerConnectInfoList = gcnew Generic::List<RemoteServerConnectInfo^>();

		/// <summary>
		/// 멀티 listen을 위한 정보
		/// </summary>
		Generic::List<MultiListenNetworkInfo^>^ m_ListneInfoList = gcnew Generic::List<MultiListenNetworkInfo^>();

		/// <summary>
		/// 중복되지 않은 프로토콜 ID 저장
		/// </summary>
		Generic::List<int>^ m_UseProtocolIDList = gcnew Generic::List<int>();
	};
}
