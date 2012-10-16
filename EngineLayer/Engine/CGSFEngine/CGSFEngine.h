#pragma once
#include "INetworkEngine.h"
#include "INetworkEngineCallback.h"
#include "ProactorAcceptor.h"
#include "ACETimeOutHandler.h"

class CGSFServerEngine : public INetworkEngine
{
public:
	CGSFServerEngine(INetworkEngineCallback* pCallback);
	virtual ~CGSFServerEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool Send(int Serial, char* pData, unsigned short Length) override;
	virtual bool Disconnect(int Serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual bool CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	int m_WorkThreadGroupID;
	ProactorAcceptor m_Acceptor;
	ACETimeOutHandler m_TimeOutHandler;
};

class CGSFClientEngine : public INetworkEngine
{
public:
	CGSFClientEngine(INetworkEngineCallback* pCallback);
	virtual ~CGSFClientEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool Send(int Serial, char* pData, unsigned short Length) override;
	virtual bool Disconnect(int Serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual bool CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	int m_WorkThreadGroupID;
	ProactorService* m_ProactorService;
	ACETimeOutHandler m_TimeOutHandler;
};