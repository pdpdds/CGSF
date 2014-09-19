// EngineLayerNET.h

#pragma once

#include "SFNETPacket.h"
#include "FRAMEWORK_ERROR_CODE.h"

using namespace System;


namespace CgsfNET64Lib {

	class ServerLogicEntry;
	class SFNETDispatcher;
	ref class ConcurrencyPacketQueue;

	public ref class CgsfNET64
	{
	public:
		CgsfNET64();
		~CgsfNET64();

		ERROR_CODE_N Init(NetworkConfig^ config);
		
		bool Start();
		void Stop();
			
		SFNETPacket^ GetPacket();

		NetworkConfig^ GetNetworkConfig() {
			return m_networkConfig;
		}

		bool SendPacket(int sessionID, UINT16 packetID, array<Byte>^ data);

		void ForceDisConnect(int sessionID);

		void LogFlush();


	private:
		void SetNetworkConfig(NetworkConfig^ config);
		

		NetworkConfig^ m_networkConfig = gcnew NetworkConfig();
		ServerLogicEntry* m_pLogicEntry = nullptr;
		SFNETDispatcher* m_pDispatcher = nullptr;
		ConcurrencyPacketQueue^ m_packetQueue;

	};
}
