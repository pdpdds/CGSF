#pragma once
#include <EngineInterface/INetworkCallback.h>

class CasualGameManager;

class TCPCallback : public INetworkCallback
{
public:
	TCPCallback(CasualGameManager* pOwner);
	virtual ~TCPCallback(void);

	virtual bool HandleNetworkMessage(BasePacket* pPacket) override;
	virtual void HandleConnect(int serial, _SessionDesc m_sessiondesc) override;
	virtual void HandleDisconnect(int serial, _SessionDesc m_sessiondesc) override;

protected:
	CasualGameManager* m_pOwner;
};

