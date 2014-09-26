#pragma once

//#include "Stdafx.h"
#include "ILogicEntry.h"
#include "ConcurrencyPacketQueue.h"

#include <gcroot.h>


//class BasePacket;

using namespace System;

namespace CgsfNET64Lib {

	class ServerLogicEntry : public ILogicEntry
	{
	public:
		ServerLogicEntry(void) {}
		virtual ~ServerLogicEntry(void) {}

		virtual bool Initialize() override
		{
			return true;
		}

		virtual bool ProcessPacket(BasePacket* pPacket) override
		{
			//LOG(INFO) << "ServerLogicEntry::ProcessPacket. PacketType: " << pPacket->GetPacketType();
			//google::FlushLogFiles(google::GLOG_INFO);

			m_refPacketQueue->Enqueue(false, pPacket);
			return true;
		}
		
		
		virtual bool SendPacket(BasePacket* pPacket)
		{
			auto result = SFEngine::GetInstance()->SendRequest(pPacket);
			return result;
		}
		


		gcroot<ConcurrencyPacketQueue^> m_refPacketQueue;


	
		
		
	};
}