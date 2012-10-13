#pragma once
#include "IProactorAgent.h"

class SFProactorService;

class SFProactorClient : public IProactorAgent
{
public:
	SFProactorClient(void);
	virtual ~SFProactorClient(void);

	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;

	virtual BOOL ServiceDisconnect(int Serial) override;
	BOOL ServiceInitialize(SFProactorService* pService);

	SFProactorService* GetNetworkService(){return m_pNetworkService;}

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) override;
	virtual BOOL Send(int Serial, SFPacket* pPacket) override;

protected:
	virtual BOOL Run() override;

private:
	SFProactorService* m_pNetworkService;
	WCHAR m_IP[20];
	USHORT m_Port;
};
