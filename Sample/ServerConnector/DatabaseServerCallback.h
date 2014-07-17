#pragma once
#include "EngineInterface/InetworkCallback.h"

class DatabaseServerCallback : public INetworkCallback
{
public:
	DatabaseServerCallback();
	virtual ~DatabaseServerCallback();

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
};

