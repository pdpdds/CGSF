#include "stdafx.h"
#include "RPCCallback.h"
#include "SFPacket.h"
#include <iostream>
#include "RPCService.h"
#include "../Common/RPCProject/testclient.h"
#include "proxy.hpp"
#include "RPCClientTransportLayer.h"



RPCCallback::RPCCallback(void)
{
}

RPCCallback::~RPCCallback(void)
{
}

bool RPCCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	
	return true;
}

bool RPCCallback::HandleRPC(BasePacket* pPacket)
{
	
	RPCClientTransportLayer trans;
	Proxy<TestInterface> proxy(trans);

	SFPacket* pSFPacket = (SFPacket*)pPacket;
	
	//std::string str("ABCDE");
	//string &retStr = proxy.invoke(&TestInterface::echo)(str);

	return true;
}
