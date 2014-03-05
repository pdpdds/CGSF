#pragma once
#include <EngineInterface/INetworkCallback.h>

class CasualGameManager;

class TCPCallback : public INetworkCallback
{
public:
	TCPCallback(CasualGameManager* pOwner);
	virtual ~TCPCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
	virtual void HandleConnect(int Serial) override;
	virtual void HandleDisconnect(int Serial) override;

protected:
	CasualGameManager* m_pOwner;
};

