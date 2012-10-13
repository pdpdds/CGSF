#include "StdAfx.h"
#include "UDPNetworkCallback.h"

UDPNetworkCallback::UDPNetworkCallback(void)
{
}

UDPNetworkCallback::~UDPNetworkCallback(void)
{
}

bool UDPNetworkCallback::HandleUDPNetworkMessage( const unsigned char* pData, unsigned int Length )
{
	return true;
}