#pragma once
#include "ISFAgent.h"

class SFMGServerReceiver;
class AsynchIOService;
class Acceptor;

class SFMGServer : public ISFAgent
{
public:
	SFMGServer(void);
	virtual ~SFMGServer(void);

	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;
	virtual BOOL Run() override;

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) override;
	virtual BOOL Send(int Serial, SFPacket* pPacket) override;


protected:

private:
	SFMGServerReceiver* m_pServerReceiver;
	AsynchIOService* m_pServer;
	Acceptor* m_pAcceptor;
};

