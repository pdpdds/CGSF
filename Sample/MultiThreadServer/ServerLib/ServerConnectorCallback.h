#pragma once

#include "EngineInterface/INetworkCallback.h"
#include "EngineInterface/EngineStructure.h"
#include "BasePacket.h"

#include "LogicEntry.h"

namespace ServerLib {
	
	/// <summary>
	/// ㅌㅌㅌㅌㅌ
	/// </summary>
	class ServerConnectorCallback : public INetworkCallback
	{
	public:
		ServerConnectorCallback(LogicEntry* pLogicEntryRef)
		{
			m_pLogicEntryRef = pLogicEntryRef;
		}

		virtual ~ServerConnectorCallback()
		{
		}

		/// <summary>
		/// 연결
		/// </summary>
		/// <param name="serial">세션 ID</param>
		/// <param name="m_sessiondesc">세션 정보</param>
		virtual void HandleConnect(int serial, _SessionDesc m_sessiondesc) override
		{
			INetworkCallback::HandleConnect(serial, m_sessiondesc);

			auto pPacket = new BasePacket;
			pPacket->SetPacketType(SFPACKET_CONNECT);
			pPacket->SetSerial = serial;
			pPacket->SetSessionDesc(m_sessiondesc);
			m_pLogicEntryRef->ProcessPacket(pPacket);
		}

		/// <summary>
		/// 연결이 끊어짐
		/// </summary>
		/// <param name="serial">세션 ID</param>
		/// <param name="m_sessiondesc">세션 정보</param>
		virtual void HandleDisconnect(int serial, _SessionDesc m_sessiondesc) override
		{
			INetworkCallback::HandleDisconnect(serial, m_sessiondesc);

			auto pPacket = new BasePacket;
			pPacket->SetPacketType(SFPACKET_DISCONNECT);
			pPacket->SetSerial = serial;
			pPacket->SetSessionDesc(m_sessiondesc);
			m_pLogicEntryRef->ProcessPacket(pPacket);
		}

		/// <summary>
		/// 패킷을 받음
		/// </summary>
		/// <param name="pPacket">네트워크 엔진을 통해서 받는 패킷 데이터</param>
		/// <returns>무저건 true 반환</returns>
		virtual bool HandleNetworkMessage(BasePacket* pPacket) override
		{
			m_pLogicEntryRef->ProcessPacket(pPacket);
			return true;
		}



	private:
		/// <summary>
		/// 
		/// </summary>
		LogicEntry* m_pLogicEntryRef = nullptr;

	};
}