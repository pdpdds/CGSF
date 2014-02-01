#pragma once
#include "INetworkCallback.h"

class TCPCallback : public INetworkCallback
{
public:
	TCPCallback(void);
	virtual ~TCPCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
	virtual void HandleConnect(int Serial) override;
	virtual void HandleDisconnect(int Serial) override;

protected:
};

