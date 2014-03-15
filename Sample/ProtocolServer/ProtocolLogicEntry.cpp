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
	m_Dispatch.RegisterMessage(PACKET_SAMPLE2, std::tr1::bind(&ProtocolLogicEntry::OnPacketSample2, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(PACKET_SAMPLE3, std::tr1::bind(&ProtocolLogicEntry::OnPacketSample3, this, std::tr1::placeholders::_1));
	m_Dispatch.RegisterMessage(PACKET_SAMPLE4, std::tr1::bind(&ProtocolLogicEntry::OnPacketSample4, this, std::tr1::placeholders::_1));

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

	printf("PACKET_SAMPLE1 received!! speed : %f, move : %f\n", speed, move);
	
	return true;
}

bool ProtocolLogicEntry::OnPacketSample2(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE2 received!!\n");
	return true;
}

bool ProtocolLogicEntry::OnPacketSample3(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE3 received!!\n");
	return true;
}

bool ProtocolLogicEntry::OnPacketSample4(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE4 received!!\n");
	return true;
}