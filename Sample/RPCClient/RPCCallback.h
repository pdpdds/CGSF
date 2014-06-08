#pragma once
#include <EngineInterface/INetworkCallback.h>

class RPCCallback : public INetworkCallback
{
public:
	RPCCallback(void);
	virtual ~RPCCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override; 
	virtual bool HandleRPC(BasePacket* pPacket) override;
};
