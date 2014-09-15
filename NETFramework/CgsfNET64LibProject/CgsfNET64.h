// EngineLayerNET.h

#pragma once

#include "SFNETPacket.h"

using namespace System;
//using namespace System::Collections::Concurrent;

namespace CgsfNET64Lib {

	class ServerLogicEntry;
	class SFNETDispatcher;
	ref class ConcurrencyPacketQueue;

	public ref class CgsfNET64
	{
	public:
		CgsfNET64();
		~CgsfNET64();

		bool Init(int threadCount, int maxBufferSize, int maxPacketSize);
		
		void Start();
		void Stop();
			
		SFNETPacket^ GetPacket();

		NetworkConfig^ GetNetworkConfig() {
			return m_networkConfig;
		}



	private:
		void SetNetworkConfigInfo();

		NetworkConfig^ m_networkConfig = gcnew NetworkConfig();
		ServerLogicEntry* m_pLogicEntry = nullptr;
		SFNETDispatcher* m_pDispatcher = nullptr;
		ConcurrencyPacketQueue^ m_packetQueue;

	};
}
