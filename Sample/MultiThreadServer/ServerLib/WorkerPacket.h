#pragma once 

#include <concrt.h>

#include "Protocol.h"


namespace ServerLib {

	class WorkerPacket
	{
	public:
		WorkerPacket() {}
		~WorkerPacket() {}

		bool IsEmpty()
		{
			return m_pBuffer == nullptr ? true : false;
		}

		void Assign(const bool IsFromInternal, const int packetType, BasePacket* pBasePacket)
		{
			m_IsFromInternal = IsFromInternal;
			m_packetType = packetType;
			m_sessionID = pBasePacket->GetSerial();
			m_serverIdentifier = pBasePacket->GetSessionDesc().identifier;

			if (packetType == SFPACKET_DATA)
			{
				auto pPacket = (SFPacket*)pBasePacket;

				m_packetID = pPacket->GetPacketID();
				
				if (pPacket->GetDataSize() > 0)
				{
					m_dataSize = pPacket->GetDataSize();
					m_pBuffer = Concurrency::Alloc(pPacket->GetDataSize());
					memcpy(m_pBuffer, pPacket->GetData(), pPacket->GetDataSize());
				}
			}
			else if (packetType == SFPACKET_CONNECT)
			{
				m_packetID = (unsigned short)PACKET_ID::SYSTEM_CONNECT;
			}
			else if (packetType == SFPACKET_DISCONNECT)
			{
				m_packetID = (unsigned short)PACKET_ID::SYSTEM_DISCONNECT;
			}
		}

		void Release()
		{
			if (m_pBuffer != nullptr)
			{
				Concurrency::Free(m_pBuffer);
			}
		}
		

		int SessionID() const { return m_sessionID; }
		int PacketType() const { return m_packetType; }
		unsigned short PacketID() const { return m_packetID; }
		void* GetData()	const { return m_pBuffer; }


	private:
		bool m_IsFromInternal = false;
		
		int m_packetType = SFPACKET_NONE;
		int m_sessionID = -1;
		unsigned short m_packetID = 0;
		unsigned short m_dataSize = 0;
		bool m_isServerConnect = false;
		short m_serverIdentifier = 0;
				
		void* m_pBuffer;
	};

}