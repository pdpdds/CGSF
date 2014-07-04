#pragma once
#include <EngineInterface/INetworkCallback.h>

class CasualGameManager;

class TCPCallback : public INetworkCallback
{
public:
	TCPCallback(CasualGameManager* pOwner);
	virtual ~TCPCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
	virtual void HandleConnect(int serial) override;
	virtual void HandleDisconnect(int serial) override;

protected:
	CasualGameManager* m_pOwner;
};

