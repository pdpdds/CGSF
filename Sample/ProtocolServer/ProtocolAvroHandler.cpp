#include "stdafx.h"
#include "ProtocolAvroHandler.h"
#include "SFAvroPacket.h"
#include "SFPacketStore/AvroProtocolPacket.hh"

ProtocolAvroHandler::ProtocolAvroHandler()
{
}


ProtocolAvroHandler::~ProtocolAvroHandler()
{
}

bool ProtocolAvroHandler::OnPacketSample1(BasePacket* pPacket)
{
	SFAvroPacket<c::Sample1>* pSample1 = (SFAvroPacket<c::Sample1>*)pPacket;

	printf("PACKET_SAMPLE1 received!! speed : %f, move : %f\n", pSample1->GetData().speed, pSample1->GetData().move);

	return true;
}

bool ProtocolAvroHandler::OnPacketSample2(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE2 received!!\n");
	return true;
}

bool ProtocolAvroHandler::OnPacketSample3(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE3 received!!\n");
	return true;
}

bool ProtocolAvroHandler::OnPacketSample4(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE4 received!!\n");
	return true;
}
