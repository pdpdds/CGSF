#pragma once

#include "SFPacket.h"
#include "DefineValue.h"

using namespace System;

namespace CgsfNET64Lib {

	/// <summary>
	/// SFPacket의 닷넷 버전
	/// </summary>
	public ref class SFNETPacket
	{
	public:
		SFNETPacket() {}
		
		/// <summary>
		/// 이 객체의 데이터로 설정한다.
		/// </summary>
		/// <param name="isServerConnect">서버간 연결 여부. true 이면 서버-서버 연결</param>
		/// <param name="pPacket">네트워크 엔진에서 받은 패킷 데이터</param>
		void SetData(bool isServerConnect, SFPacket* pPacket)
		{
			m_isServerConnect = isServerConnect;
			if (m_isServerConnect)
			{
				m_serverIdentifier = (short)pPacket->GetSessionDesc().identifier;
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

		/// <summary>
		/// 이 객체의 데이터로 설정한다. 내부 패킷을 만들 때 사용
		/// </summary>
		/// <param name="sessionID">세션</param>
		/// <param name="packetID">패킷 ID</param>
		/// <param name="data">패킷 데이터</param>
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

		/// <summary>
		/// 이 객체의 데이터로 설정한다. 주로 연결/끈어짐 관련 내부 패킷을 만들 때 사용
		/// </summary>
		/// <param name="packetType">패킷 타입</param>
		/// <param name="sessionID">세션 ID</param>
		/// <param name="identifier">서버간 연결인 경우 어느 서버인지 알 수 있다.</param>
		void SetSystemData(SFPACKET_TYPE packetType, int sessionID, int identifier)
		{
			m_isServerConnect = true;
			m_serverIdentifier = (short)identifier;
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
		short ServerIdentifier() { return m_serverIdentifier; }


	private:
		int m_sessionID;
		
		unsigned short m_packetID;
		unsigned long  m_packetOption;
		unsigned long  m_dataCRC;
		unsigned short m_dataSize;

		bool m_isServerConnect = false;
		short m_serverIdentifier = 0;

		SFPACKET_TYPE m_packetType;
		array<Byte>^ m_packetData;
	};

	
}