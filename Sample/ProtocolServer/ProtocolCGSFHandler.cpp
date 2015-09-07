#include "stdafx.h"
#include "ProtocolCGSFHandler.h"
#include "SFPacket.h"

ProtocolCGSFHandler::ProtocolCGSFHandler()
{
}


ProtocolCGSFHandler::~ProtocolCGSFHandler()
{
}

bool ProtocolCGSFHandler::OnPacketSample1(BasePacket* pPacket)
{
	SFPacket* pSFPacket = (SFPacket*)pPacket;

	float speed = 0.0f;
	float move = 0.0f;

	*pSFPacket >> speed >> move;

	printf("PACKET_SAMPLE1 received!! speed : %f, move : %f\n", speed, move);

	return true;
}

bool ProtocolCGSFHandler::OnPacketSample2(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE2 received!!\n");
	return true;
}

bool ProtocolCGSFHandler::OnPacketSample3(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE3 received!!\n");
	return true;
}

bool ProtocolCGSFHandler::OnPacketSample4(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE4 received!!\n");
	return true;
}