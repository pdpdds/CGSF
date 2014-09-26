#pragma once

#include "SFPacket.h"
#include "DefineValue.h"

using namespace System;

namespace CgsfNET64Lib {

	public ref class SFNETPacket
	{
	public:
		SFNETPacket() {}
		
		void SetData(bool isServerConnect, SFPacket* pPacket)
		{
			m_isServerConnect = isServerConnect;
			if (m_isServerConnect)
			{
				m_serverConnectorID = (short)pPacket->GetSessionDesc().identifier;
			}

			m_packetType = (SFPACKET_TYPE)pPacket->GetPacketType();
			m_sessionID = pPacket->GetSerial();

			if (m_packetType == SFPACKET_TYPE::CONNECT || m_packetType == SFPACKET_TYPE::DISCONNECT ||
				m_packetType == SFPACKET_TYPE::TIMER || m_packetType == SFPACKET_TYPE::SERVERSHUTDOWN)
			{
				return;
			}
			
			auto pHeader = pPacket->GetHeader();
			m_packetID = pHeader->packetID;
			m_packetOption = pHeader->packetOption;
			m_dataCRC = pHeader->dataCRC;
			m_dataSize = pHeader->dataSize;
			m_packetData = nullptr;

			if (0 < m_dataSize)
			{
				m_packetData = gcnew array<Byte>(m_dataSize);
				System::Runtime::InteropServices::Marshal::Copy((IntPtr)pPacket->GetData(), m_packetData, 0, m_dataSize);
			}
		}

		void SetData(int sessionID, unsigned short packetID, array<Byte>^ data)
		{
			m_packetType = SFPACKET_TYPE::DATA;

			m_sessionID = sessionID;
			m_packetID = packetID;
			m_packetOption = 0;
			m_dataCRC = 0;

			if (data != nullptr)
			{
				m_dataSize = data->Length;
				m_packetData = data;
			}
			else
			{ 
				m_dataSize = 0;
			}
		}

		void SetData(SFPACKET_TYPE packetType, int sessionID, int identifier)
		{
			m_isServerConnect = true;
			m_serverConnectorID = (short)identifier;
			m_packetType = packetType;
			m_sessionID = sessionID;
		}

		SFPACKET_TYPE GetPacketType() { return m_packetType; }
		
		int SessionID() { return m_sessionID; }
		unsigned short PacketID() { return m_packetID; }
		unsigned long  packetOption() { return m_packetOption; }
		unsigned long  CRC() { return m_dataCRC; }
		unsigned short Size() { return m_dataSize; }
		array<Byte>^ GetData() { return m_packetData;  }

		bool IsServerConnect() { return m_isServerConnect; }
		short ServerConnectorID() { return m_serverConnectorID; }


	private:
		int m_sessionID;
		
		unsigned short m_packetID;
		unsigned long  m_packetOption;
		unsigned long  m_dataCRC;
		unsigned short m_dataSize;

		bool m_isServerConnect = false;
		short m_serverConnectorID = 0;

		SFPACKET_TYPE m_packetType;
		array<Byte>^ m_packetData;
	};

	
}