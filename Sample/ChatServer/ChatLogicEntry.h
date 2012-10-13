#pragma once
#include "ILogicEntry.h"
#include <map>

class SFCommand;
class ChatUser;
class SFPacket;

class ChatLogicEntry : public ILogicEntry
{
	typedef std::map<int, ChatUser*> ChatUserMap;

public:
	ChatLogicEntry(void);
	virtual ~ChatLogicEntry(void);

	virtual BOOL ProcessPacket(SFCommand* pCommand) override;
	BOOL Send(int Serial, SFPacket* pPacket);
	BOOL Broadcast(SFPacket* pPacket, int Serial = -1);

protected:
	BOOL OnConnectPlayer(int Serial);
	BOOL OnPlayerData(SFPacket* pPacket);
	BOOL OnDisconnectPlayer(int Serial);

private:
	ChatUserMap m_ChatUserMap;
};
