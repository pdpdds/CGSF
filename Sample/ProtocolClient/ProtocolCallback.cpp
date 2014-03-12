#include "stdafx.h"
#include "ProtocolCallback.h"
#include "SFJsonPacket.h"
#include <iostream>

ProtocolCallback::ProtocolCallback(void)
{
}

ProtocolCallback::~ProtocolCallback(void)
{
}

bool ProtocolCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	//SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

	//std::cout << "Received : " << pJsonPacket->GetData().GetValue<tstring>("ECHO") << std::endl;

	return true;
}