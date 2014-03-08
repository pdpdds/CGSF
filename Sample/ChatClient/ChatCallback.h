#pragma once
#include <EngineInterface/INetworkCallback.h>

class ChatCallback : public INetworkCallback
{
public:
	ChatCallback(void);
	virtual ~ChatCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
};