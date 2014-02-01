#pragma once
#include "INetworkCallback.h"

class EchoCallback : public INetworkCallback
{
public:
	EchoCallback(void);
	virtual ~EchoCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override; 
};

