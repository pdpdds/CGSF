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

	virtual BOOL ServiceDisconnect(int Serial) override;
	virtual BOOL ServiceInitialize(SFProactorService* pService) override;

	BOOL AddTimer(DWORD TimerID, DWORD StartTime, DWORD Period);

protected:
	BOOL CreateTimerTask( ILogicEntry* pLogic);
	virtual BOOL Run() override;

private:
	SFProactorAcceptor m_Acceptor;
	SFACETimeOutHandler m_TimeOutHandler;
};
