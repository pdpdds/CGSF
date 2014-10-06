#pragma once 

#include "stdafx.h"
#include "SFNETPacket.h"


using namespace System;
using namespace System::Collections::Concurrent;

namespace CgsfNET64Lib {

	/// <summary>
	/// 스레드 세이프한 패킷 저장 큐
	/// </summary>
	ref class ConcurrencyPacketQueue
	{
	public:
		ConcurrencyPacketQueue() {}
		~ConcurrencyPacketQueue() {}

		/// <summary>
		/// 큐에 패킷을 넣는다.
		/// </summary>
		/// <param name="isServerConnect">true이면 서버간 세션</param>
		/// <param name="pPacket">네트워크 엔진을 통해서 받은 패킷. C++ 타입</param>
		void Enqueue(bool isServerConnect, BasePacket* pPacket)
		{
			auto packet = MakeNetPacket(isServerConnect, pPacket);
			m_packetQueue->Enqueue(packet);
		}

		/// <summary>
		/// 큐에 패킷을 넣는다. 네트워크 엔진이 아닌 내부에서 넣는 경우
		/// </summary>
		/// <param name="packet">패킷. .NET 타입</param>
		void Enqueue(SFNETPacket^ packet)
		{
			m_packetQueue->Enqueue(packet);
		}

		/// <summary>
		/// 큐에서 패킷을 꺼낸다.
		/// </summary>
		/// <returns>패킷이 없다면 null 반환.</returns>
		SFNETPacket^ Dequeue()
		{
			SFNETPacket^ packet = nullptr;

			m_packetQueue->TryDequeue(packet);

			return packet;
		}
		

	private:
		/// <summary>
		/// C++ 타입의 패킷을 .NET용 타입의 패킷으로 만든다.
		/// </summary>
		/// <param name="isServerConnect">true이면 서버간 세션</param>
		/// <param name="pPacket">네트워크 엔진을 통해서 받은 패킷. C++ 타입</param>
		/// <returns>패킷. .NET 타입</returns>
		SFNETPacket^ MakeNetPacket(bool isServerConnect, BasePacket* pPacket)
		{
			SFPacket* pSFPacket = (SFPacket*)pPacket;

			auto packet = gcnew SFNETPacket();
			packet->SetData(isServerConnect, pSFPacket);

			return packet;
		}
		
		/// <summary>
		/// 패킷 큐
		/// </summary>
		ConcurrentQueue<SFNETPacket^>^ m_packetQueue = gcnew ConcurrentQueue<SFNETPacket^>();
	};
}