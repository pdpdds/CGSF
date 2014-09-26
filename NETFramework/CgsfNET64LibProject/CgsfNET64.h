// EngineLayerNET.h

#pragma once

#include "SFNETPacket.h"
#include "FRAMEWORK_ERROR_CODE.h"

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

		NET_ERROR_CODE_N Init(NetworkConfig^ config);
		
		bool Start();
		void Stop();
			
		SFNETPacket^ GetPacket();

		void InnerPacket(SFNETPacket^ packet);

		NetworkConfig^ GetNetworkConfig() {
			return m_networkConfig;
		}

		bool SendPacket(int sessionID, UINT16 packetID, array<Byte>^ data);

		void ForceDisConnect(int sessionID);

		void LogFlush();


		NET_ERROR_CODE_N RegistConnectInfo(RemoteServerConnectInfo^ connectInfo);
		bool SetupServerReconnectSys();


	private:
		void SetNetworkConfig(NetworkConfig^ config);
		

		NetworkConfig^ m_networkConfig = gcnew NetworkConfig();
		
		ServerLogicEntry* m_pLogicEntry = nullptr;
		SFNETDispatcher* m_pDispatcher = nullptr;
		ServerConnectCallback* m_pServerConnectCallback = nullptr;

		ConcurrencyPacketQueue^ m_packetQueue;

		Generic::List<RemoteServerConnectInfo^>^ m_RemoteServerConnectInfoList = gcnew Generic::List<RemoteServerConnectInfo^>();
	};
}
