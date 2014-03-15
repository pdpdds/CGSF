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
	return true;
}