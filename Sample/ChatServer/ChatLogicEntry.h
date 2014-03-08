#pragma once
#include "ILogicEntry.h"
#include <map>

class ChatUser;
class BasePacket;

class ChatLogicEntry : public ILogicEntry
{
	typedef std::map<int, ChatUser*> ChatUserMap;

public:
	ChatLogicEntry(void);
	virtual ~ChatLogicEntry(void);

	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
	BOOL SendRequest(BasePacket* pPacket);
	BOOL Broadcast(BasePacket* pPacket);

protected:
	BOOL OnConnectPlayer(int Serial);
	BOOL OnPlayerData(BasePacket* pPacket);
	BOOL OnDisconnectPlayer(int Serial);

private:
	ChatUserMap m_ChatUserMap;
};
