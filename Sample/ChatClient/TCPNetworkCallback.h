#pragma once
#include "INetworkCallback.h"

class TCPNetworkCallback : public INetworkCallback
{
public:
	TCPNetworkCallback(void);
	virtual ~TCPNetworkCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
};