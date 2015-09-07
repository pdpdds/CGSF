#pragma once
#include <EngineInterface/INetworkEngine.h>
#include "ASIOServer.h"
#include "ASIOClient.h"

const int MAX_SESSION_COUNT = 100;

class ASIOServerEngine : public INetworkEngine
{
public:
	ASIOServerEngine(IEngine* pEngine);
	virtual ~ASIOServerEngine(void);

	virtual bool Init(int ioThreadCnt) override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool SendRequest(BasePacket* pPacket) override;

	virtual bool Disconnect(int serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual long AddTimer(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	boost::asio::io_service io_service;
	ASIOServer* m_pServer;
};

class ASIOClientEngine : public INetworkEngine
{
public:
	ASIOClientEngine(IEngine* pEngine);
	virtual ~ASIOClientEngine(void);

	virtual bool Init(int ioThreadCnt) override;
	virtual bool Start(char* szIP, unsigned short port) override;
    virtual bool Shutdown() override;

	virtual bool SendRequest(BasePacket* pPacket) override;

	virtual bool Disconnect(int serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual long AddTimer(unsigned int timerID, unsigned int startTime, unsigned int period) override;

protected:

private:
	boost::asio::io_service io_service;
	ASIOClient* m_pClient;
};

