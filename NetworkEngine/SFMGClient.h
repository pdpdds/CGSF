#pragma once
#include "ISFAgent.h"

class SFMGClientReceiver;
class AsynchIOService;

class SFMGClient : public ISFAgent
{
public:
	SFMGClient(void);
	virtual ~SFMGClient(void);

	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;
	virtual BOOL Run() override;

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) override;
	virtual BOOL Send(int Serial, SFPacket* pPacket) override;

protected:
	
private:
	SFMGClientReceiver* m_pClientReceiver;
	AsynchIOService* m_pService;
};

