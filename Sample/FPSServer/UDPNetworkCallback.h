#pragma once
#include "IUDPNetworkCallback.h"

class UDPNetworkCallback : public IUDPNetworkCallback
{
public:
	UDPNetworkCallback(void);
	virtual ~UDPNetworkCallback(void);

	virtual bool HandleUDPNetworkMessage(const unsigned char* pData, unsigned int Length) override;
};
