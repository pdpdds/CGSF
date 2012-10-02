#pragma once
#include "ILogicEntry.h"

class SFCommand;
class SFPacket;

class ChatPacketEntry : public ILogicEntry
{
public:
	ChatPacketEntry(void);
	virtual ~ChatPacketEntry(void);

	virtual BOOL ProcessPacket(SFCommand* pCommand) override;

	BOOL Send(int Serial, SFPacket* pPacket);
	BOOL Send(USHORT PacketID, char* pBuffer, int BufSize );
	BOOL Send(int Serial, USHORT PacketID, char* pBuffer, int BufSize );

	int GetSerial(){return m_Serial;}

protected:

private:
	int m_Serial;
};