#pragma once

#include "EngineInterface/INetworkCallback.h"
#include "ConcurrencyPacketQueue.h"
#include <gcroot.h>


namespace CgsfNET64Lib {

	/// <summary>
	/// ㅌㅌㅌㅌㅌ
	/// </summary>
	class ServerConnectCallback : public INetworkCallback
	{
	public:
		ServerConnectCallback()
		{
		}

		virtual ~ServerConnectCallback()
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

			SFNETPacket^ packet = gcnew SFNETPacket();
			packet->SetSystemData(SFPACKET_TYPE::CONNECT, serial, m_sessiondesc.identifier);
			m_refPacketQueue->Enqueue(packet);
		}

		/// <summary>
		/// 연결이 끊어짐
		/// </summary>
		/// <param name="serial">세션 ID</param>
		/// <param name="m_sessiondesc">세션 정보</param>
		virtual void HandleDisconnect(int serial, _SessionDesc m_sessiondesc) override
		{
			INetworkCallback::HandleDisconnect(serial, m_sessiondesc);

			SFNETPacket^ packet = gcnew SFNETPacket();
			packet->SetSystemData(SFPACKET_TYPE::DISCONNECT, serial, m_sessiondesc.identifier);
			m_refPacketQueue->Enqueue(packet);
		}

		/// <summary>
		/// 패킷을 받음
		/// </summary>
		/// <param name="pPacket">네트워크 엔진을 통해서 받는 패킷 데이터</param>
		/// <returns>무저건 true 반환</returns>
		virtual bool HandleNetworkMessage(BasePacket* pPacket) override
		{
			m_refPacketQueue->Enqueue(true, pPacket);
			return true;
		}
		


		/// <summary>
		/// 패킷 큐. 외부에서 할당 받는다.
		/// </summary>
		gcroot<ConcurrencyPacketQueue^> m_refPacketQueue;

	};
}