#pragma once
#include "ILogicEntry.h"
#include "VMemPool.h"

class BasePacket;

class ChatPacketEntry : public ILogicEntry,  public CVMemPool<ChatPacketEntry>
{
public:
	ChatPacketEntry(void);
	virtual ~ChatPacketEntry(void);

	virtual BOOL ProcessPacket(BasePacket* pPacket) override;

	BOOL SendRequest(BasePacket* pPacket);

	int GetSerial(){return m_Serial;}

protected:

private:
	int m_Serial;
};
