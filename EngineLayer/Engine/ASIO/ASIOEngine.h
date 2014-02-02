#pragma once
#include "INetworkEngine.h"
#include "ChattingServer.h"
#include "ChattingClient.h"

const int MAX_SESSION_COUNT = 100;

class ASIOServerEngine : public INetworkEngine
{
public:
	ASIOServerEngine(IEngine* pEngine);
	virtual ~ASIOServerEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool SendRequest(BasePacket* pPacket) override;

	virtual bool Disconnect(int Serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual bool CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	boost::asio::io_service io_service;
	ChatServer* m_pServer;
};

class ASIOClientEngine : public INetworkEngine
{
public:
	ASIOClientEngine(IEngine* pEngine);
	virtual ~ASIOClientEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool SendRequest(BasePacket* pPacket) override;

	virtual bool Disconnect(int Serial) override;

	virtual bool CheckTimerImpl() override; //default timer 사용
	virtual bool CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period) override;

protected:

private:
	boost::asio::io_service io_service;
	ChatClient* m_pClient;
};

