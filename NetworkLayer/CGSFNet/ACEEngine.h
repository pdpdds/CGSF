#pragma once
#include <EngineInterface\/INetworkEngine.h>
#include "ProactorAcceptor.h"
#include "ACETimeOutHandler.h"
#include <map>

typedef std::map<int, ProactorAcceptor*> mapAcceptor;
typedef std::map<int, ProactorService*> mapConnector;

class ACEEngine : public INetworkEngine
{
public:
	ACEEngine(IEngine* pEngine);
	virtual ~ACEEngine(void);

	virtual bool Init() override;
    virtual bool Shutdown() override;

	virtual bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize) override;

	virtual bool Disconnect(int serial) override;

	virtual bool CheckTimerImpl() override; //default timer »ç¿ë
	virtual bool CreateTimerTask(unsigned int timerID, unsigned int startTime, unsigned int period) override;

	virtual int AddConnector(char* szIP, unsigned short port) override;
	virtual int AddListener(char* szIP, unsigned short port) override;

protected:
	int m_acceptorIndex;
	int m_workThreadGroupID;
	mapAcceptor m_mapAcceptor;
	mapConnector m_mapConnector;

private:
	ACETimeOutHandler m_TimeOutHandler;
};

class ACEServerEngine : public ACEEngine
{
public:
	ACEServerEngine(IEngine* pEngine);
	virtual bool Start(char* szIP, unsigned short port) override;
};

class ACEClientEngine : public ACEEngine
{
public:
	ACEClientEngine(IEngine* pEngine);
	virtual bool Start(char* szIP, unsigned short port) override;
};

