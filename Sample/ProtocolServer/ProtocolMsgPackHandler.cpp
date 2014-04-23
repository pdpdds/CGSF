#include "stdafx.h"
#include "ProtocolMsgPackHandler.h"
#include "msgpack.hpp"
#include "SFMsgPackPacket.h"
#include <iostream>

ProtocolMsgPackHandler::ProtocolMsgPackHandler()
{
}


ProtocolMsgPackHandler::~ProtocolMsgPackHandler()
{
}

bool ProtocolMsgPackHandler::OnPacketSample1(BasePacket* pPacket)
{
	SFMsgPackPacket* pSFPacket = (SFMsgPackPacket*)pPacket;

	msgpack::unpacked result;
	while (pSFPacket->GetData().next(&result)) {
		std::cout << result.get() << std::endl;
	}

	return true;
}

bool ProtocolMsgPackHandler::OnPacketSample2(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE2 received!!\n");
	return true;
}

bool ProtocolMsgPackHandler::OnPacketSample3(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE3 received!!\n");
	return true;
}

bool ProtocolMsgPackHandler::OnPacketSample4(BasePacket* pPacket)
{
	printf("PACKET_SAMPLE4 received!!\n");
	return true;
}