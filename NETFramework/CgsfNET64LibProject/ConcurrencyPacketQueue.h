#pragma once 

#include "stdafx.h"
#include "SFNETPacket.h"


using namespace System;
using namespace System::Collections::Concurrent;

namespace CgsfNET64Lib {

	ref class ConcurrencyPacketQueue
	{
	public:
		ConcurrencyPacketQueue() {}
		~ConcurrencyPacketQueue() {}

		void Enqueue(bool isServerConnect, BasePacket* pPacket)
		{
			SFPacket* pSFPacket = (SFPacket*)pPacket;

			auto packet = gcnew SFNETPacket();
			packet->SetData(isServerConnect, pSFPacket);

			m_packetQueue->Enqueue(packet);
		}

		void Enqueue(SFNETPacket^ packet)
		{
			m_packetQueue->Enqueue(packet);
		}

		SFNETPacket^ Dequeue()
		{
			SFNETPacket^ packet = nullptr;

			m_packetQueue->TryDequeue(packet);

			return packet;
		}

	private:
		ConcurrentQueue<SFNETPacket^>^ m_packetQueue = gcnew ConcurrentQueue<SFNETPacket^>();
	};
}