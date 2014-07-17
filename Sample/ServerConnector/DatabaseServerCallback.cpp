#include "stdafx.h"
#include "DatabaseServerCallback.h"


DatabaseServerCallback::DatabaseServerCallback()
{
}


DatabaseServerCallback::~DatabaseServerCallback()
{
}

bool DatabaseServerCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	return true;
}
