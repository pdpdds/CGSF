#pragma once
#include "IProactorAgent.h"
#include "SFProactorAcceptor.h"
#include "SFPacket.h"
#include "SFTSSyncQueue.h"
#include "SFIOCPQueue.h"
#include "SFACETimeOutHandler.h"

class SFProactorServer : public IProactorAgent
{
public:
	SFProactorServer(void);
	virtual ~SFProactorServer(void);

	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;
	virtual BOOL Run() override;

	virtual BOOL ServiceDisconnect(int Serial) override;
	virtual BOOL ServiceInitialize(SFProactorService* pService) override;

	BOOL AddTimer(DWORD TimerID, DWORD StartTime, DWORD Period);

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) override;
	virtual BOOL Send(int Serial, SFPacket* pPacket) override;

protected:
	BOOL CreateTimerTask( ILogicEntry* pLogic);
	

private:
	SFProactorAcceptor m_Acceptor;
	SFACETimeOutHandler m_TimeOutHandler;
};
