#include "stdafx.h"
#include "AuthServerCallback.h"


AuthServerCallback::AuthServerCallback()
{
}


AuthServerCallback::~AuthServerCallback()
{
}

bool AuthServerCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	return true;
}