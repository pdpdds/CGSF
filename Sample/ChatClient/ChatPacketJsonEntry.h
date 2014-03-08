#pragma once
#include "ILogicEntry.h"
#include <VMemPool/VMemPool.h>

class BasePacket;

class ChatPacketJsonEntry : public ILogicEntry,  public CVMemPool<ChatPacketJsonEntry>
{
public:
	ChatPacketJsonEntry(void);
	virtual ~ChatPacketJsonEntry(void);

	virtual bool ProcessPacket(BasePacket* pPacket) override;

	BOOL SendRequest(BasePacket* pPacket);

	int GetSerial(){return m_Serial;}

protected:
	BOOL OnConnectPlayer( int Serial );
	BOOL OnDisconnectPlayer( int Serial );
	BOOL OnPlayerData( BasePacket* pPacket );

private:
	int m_Serial;
};
