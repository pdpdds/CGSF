#include "stdafx.h"
#include "ProtocolLogicEntry.h"
#include "SFJsonPacket.h"
#include "SFMessage.h"

#define PACKET_SAMPLE1 1000
#define PACKET_SAMPLE2 1001
#define PACKET_SAMPLE3 1002
#define PACKET_SAMPLE4 1003

ProtocolLogicEntry::ProtocolLogicEntry(void)
{
}

ProtocolLogicEntry::~ProtocolLogicEntry(void)
{
}

bool ProtocolLogicEntry::Initialize()
{
	m_Dispatch.RegisterMessage(PACKET_SAMPLE1, std::tr1::bind(&ProtocolLogicEntry::OnPacketSample1, this, std::tr1::placeholders::_1));

	return true;
}

bool ProtocolLogicEntry::ProcessPacket(BasePacket* pPacket)
{
	switch (pPacket->GetPacketType())
	{	
	case SFPACKET_DATA:
		return m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPacket);
	}

	return true;
}

bool ProtocolLogicEntry::OnPacketSample1(BasePacket* pPacket)
{
	SFPacket* pSFPacket = (SFPacket*)pPacket;

	float speed = 0.0f;
	float move = 0.0f;
	
	*pSFPacket >> speed >> move;
	
	return true;
}

bool ProtocolLogicEntry::OnPacketSample2(BasePacket* pPacket)
{
	return true;
}

bool ProtocolLogicEntry::OnPacketSample3(BasePacket* pPacket)
{
	return true;
}

bool ProtocolLogicEntry::OnPacketSample4(BasePacket* pPacket)
{
	return true;
}