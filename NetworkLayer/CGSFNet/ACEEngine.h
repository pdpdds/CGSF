#pragma once
#include <EngineInterface\/INetworkEngine.h>
#include "ProactorAcceptor.h"
#include "ACETimeOutHandler.h"

class ACEServerEngine : public INetworkEngine
{
public:
	ACEServerEngine(IEngine* pEngine);
	virtual ~ACEServerEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize) override;

	virtual bool Disconnect(int Serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual bool CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	int m_workThreadGroupID;
	ProactorAcceptor m_Acceptor;
	ACETimeOutHandler m_TimeOutHandler;
};

class ACEClientEngine : public INetworkEngine
{
public:
	ACEClientEngine(IEngine* pEngine);
	virtual ~ACEClientEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize) override;
	virtual bool Disconnect(int Serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual bool CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	int m_workThreadGroupID;
	ProactorService* m_ProactorService;
	ACETimeOutHandler m_TimeOutHandler;
};

