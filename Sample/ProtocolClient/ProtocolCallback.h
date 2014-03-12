#pragma once
#include <EngineInterface/INetworkCallback.h>

class ProtocolCallback : public INetworkCallback
{
public:
	ProtocolCallback(void);
	virtual ~ProtocolCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override; 
};
