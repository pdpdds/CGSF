#pragma once
#include "ILogicEntry.h"
#include <VMemPool/VMemPool.h>

class BasePacket;

class ChatPacketEntry : public ILogicEntry,  public CVMemPool<ChatPacketEntry>
{
public:
	ChatPacketEntry(void);
	virtual ~ChatPacketEntry(void);

	virtual bool ProcessPacket(BasePacket* pPacket) override;

	BOOL SendRequest(BasePacket* pPacket);

	int GetSerial(){return m_Serial;}

protected:

private:
	int m_Serial;
};
