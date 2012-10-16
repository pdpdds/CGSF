#pragma once
#include "INetworkEngineCallback.h"
#include "SFLock.h"
#include <map>

class SFUserSession;

class SFNetworkEngineCallback : public INetworkEngineCallback
{
	typedef std::map<int, SFUserSession*> UserSessionMap;
public:
	SFNetworkEngineCallback(void);
	virtual ~SFNetworkEngineCallback(void);

	virtual bool OnConnect(int Serial) override;
	virtual bool OnDisconnect(int Serial) override;
	virtual bool OnData(int Serial, char* pData, unsigned short Length) override;
	virtual bool OnTimer(const void *arg) override;

protected:

private:
	UserSessionMap m_UserSessionMap;
	SFLock m_Lock;
};

