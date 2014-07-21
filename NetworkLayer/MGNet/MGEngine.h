#pragma once
#include <EngineInterface/INetworkEngine.h>

class MGServerReceiver;
class MGClientReceiver;
class AsynchIOService;
class Acceptor;
class IEngine;

class MGServerEngine : public INetworkEngine
{
public:
	MGServerEngine(IEngine* pEngine);
	virtual ~MGServerEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short Port) override;
    virtual bool Shutdown() override;

	virtual bool SendRequest(BasePacket* pPacket) override;
	virtual bool Disconnect(int serial) override;

protected:

private:
	MGServerReceiver* m_pServerReceiver;
	AsynchIOService* m_pServer;
	Acceptor* m_pAcceptor;
};

class MGClientEngine : public INetworkEngine
{
public:
	MGClientEngine(IEngine* pEngine);
	virtual ~MGClientEngine(void);

	virtual bool Init() override;
	virtual bool Start(char* szIP, unsigned short port) override;
    virtual bool Shutdown() override;

	virtual bool SendRequest(BasePacket* pPacket) override;
	virtual bool Disconnect(int serial) override;

protected:

private:
	MGClientReceiver* m_pClientReceiver;
	AsynchIOService* m_pService;
};