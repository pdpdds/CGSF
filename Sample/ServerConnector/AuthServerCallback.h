#pragma once
#include "EngineInterface/INetworkCallback.h"

class AuthServerCallback : public INetworkCallback
{
public:
	AuthServerCallback();
	virtual ~AuthServerCallback();

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
};

