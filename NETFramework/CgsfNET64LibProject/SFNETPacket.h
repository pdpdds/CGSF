#pragma once

#include "SFPacket.h"
#include "DefineValue.h"

using namespace System;

namespace CgsfNET64Lib {

	public ref class SFNETPacket
	{
	public:
		SFNETPacket() {}
		
		void SetData(SFPacket* pPacket)
		{
			m_packetType = (SFPACKET_TYPE)pPacket->GetPacketType();
			m_serial = pPacket->GetSerial();

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


		SFPACKET_TYPE GetPacketType() { return m_packetType; }
		
		int Serial() { return m_serial; }
		unsigned short PacketID() { return m_packetID; }
		unsigned long  packetOption() { return m_packetOption; }
		unsigned long  CRC() { return m_dataCRC; }
		unsigned short Size() { return m_dataSize; }
		array<Byte>^ GetData() { return m_packetData;  }


	private:
		int m_serial;
		
		unsigned short m_packetID;
		unsigned long  m_packetOption;
		unsigned long  m_dataCRC;
		unsigned short m_dataSize;

		SFPACKET_TYPE m_packetType;
		array<Byte>^ m_packetData;
	};

	
}