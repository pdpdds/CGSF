#include "stdafx.h"
#include "SFUserSession.h"


SFUserSession::SFUserSession(int Serial)
	: m_Serial(Serial)
{
}


SFUserSession::~SFUserSession(void)
{
}

bool SFUserSession::ProcessData(char* pData, unsigned short Length)
{
	m_BufferAnalyzer.AddTransferredData(pData, Length);

	int ErrorCode = PACKETIO_ERROR_NONE;

	SFPacket* pPacket = PacketPoolSingleton::instance()->Alloc();

	while(TRUE ==  m_BufferAnalyzer.GetPacket(*pPacket, ErrorCode))
	{
		pPacket->SetPacketType(SFCommand_Data);
		pPacket->SetOwnerSerial(m_Serial);

		LogicGatewaySingleton::instance()->PushPacket(pPacket);

		pPacket = PacketPoolSingleton::instance()->Alloc();
		pPacket->Initialize();
	}

	PacketPoolSingleton::instance()->Release(pPacket);
	
	if(ErrorCode != PACKETIO_ERROR_NONE)
	{
		return false;
	}
	return true;
}
