#pragma once

#include "ILogicEntry.h"
#include "ConcurrencyPacketQueue.h"

#include <gcroot.h>
#include <map>

class BasePacket;

using namespace System;

namespace CgsfNET64Lib {

	class ServerLogicEntry : public ILogicEntry
	{
	public:
		ServerLogicEntry(void) {}
		virtual ~ServerLogicEntry(void) {}

		virtual bool Initialize() override;		
		virtual bool ProcessPacket(BasePacket* pBasePacket) override;
		
		
		virtual bool SendPacket(BasePacket* pPacket);
		
		gcroot<ConcurrencyPacketQueue^> m_refPacketQueue;


	private:
		
		//bool OnConnect(int serial);
		//bool OnProcessData(BasePacket* pPacket);
		//bool OnDisconnect(int serial);
	};
}