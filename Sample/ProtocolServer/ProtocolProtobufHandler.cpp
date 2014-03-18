#include "stdafx.h"
#include "ProtocolProtobufHandler.h"
#include "SFProtobufPacket.h"

ProtocolProtobufHandler::ProtocolProtobufHandler()
{
}


ProtocolProtobufHandler::~ProtocolProtobufHandler()
{
}

bool ProtocolProtobufHandler::OnPacketSample1(BasePacket* pPacket)
{
	SFProtobufPacket<ProtocolPacket::Sample1>* pSample1 = (SFProtobufPacket<ProtocolPacket::Sample1>*)pPacket;

	printf("PACKET_SAMPLE1 received!! speed : %f, move : %f\n", pSample1->GetData().speed(), pSample1->GetData().move());

	return true;
}

bool ProtocolProtobufHandler::OnPacketSample2(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE2 received!!\n");
	return true;
}

bool ProtocolProtobufHandler::OnPacketSample3(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE3 received!!\n");
	return true;
}

bool ProtocolProtobufHandler::OnPacketSample4(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE4 received!!\n");
	return true;
}