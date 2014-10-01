#pragma once

//#include "Stdafx.h"
#include "ILogicEntry.h"
#include "ConcurrencyPacketQueue.h"

#include <gcroot.h>


//class BasePacket;

using namespace System;

namespace CgsfNET64Lib {

	/// <summary>
	/// 네트워크 처리를 하는 클래스
	/// </summary>
	class ServerLogicEntry : public ILogicEntry
	{
	public:
		ServerLogicEntry(void) {}
		virtual ~ServerLogicEntry(void) {}

		/// <summary>
		/// 초기화. 아무런 동작 하지 않음
		/// </summary>
		/// <returns>무저건 true 반환</returns>
		virtual bool Initialize() override
		{
			return true;
		}

		/// <summary>
		/// 패킷을 받음
		/// </summary>
		/// <param name="pPacket">네트워크 엔진을 통해서 받는 패킷 데이터</param>
		/// <returns>무저건 true 반환</returns>
		virtual bool ProcessPacket(BasePacket* pPacket) override
		{
			m_refPacketQueue->Enqueue(false, pPacket);
			return true;
		}
		
		/// <summary>
		/// 패킷을 보낸다
		/// </summary>
		/// <param name="pPacket">네트워크 엔진을 통해 보낼 패킷 데이터</param>
		/// <returns>true가 아니면 보내기 실패</returns>
		virtual bool SendPacket(BasePacket* pPacket)
		{
			auto result = SFEngine::GetInstance()->SendRequest(pPacket);
			return result;
		}
		

		/// <summary>
		/// 패킷 큐. 외부에서 할당 받는다.
		/// </summary>
		gcroot<ConcurrencyPacketQueue^> m_refPacketQueue;
		
	};
}