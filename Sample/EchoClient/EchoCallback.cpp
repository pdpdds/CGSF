#include "stdafx.h"
#include "EchoCallback.h"
#include "SFJsonPacket.h"
#include <iostream>

EchoCallback::EchoCallback(void)
{
}

EchoCallback::~EchoCallback(void)
{
}

bool EchoCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	SFJsonPacket* pJsonPacket = (SFJsonPacket*)pPacket;

	std::cout << "Received : " << pJsonPacket->GetData().GetValue<tstring>("ECHO") << std::endl;

	return TRUE;
}