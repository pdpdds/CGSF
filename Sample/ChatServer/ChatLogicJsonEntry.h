#pragma once
#include "ILogicEntry.h"
#include <map>

class ChatUser;
class BasePacket;

class ChatLogicJsonEntry : public ILogicEntry
{
	typedef std::map<int, ChatUser*> ChatUserMap;
public:
	ChatLogicJsonEntry(void);
	virtual ~ChatLogicJsonEntry(void);

	virtual BOOL Initialize() override;
	virtual BOOL ProcessPacket(BasePacket* pBasePacket) override;
	BOOL SendRequest(BasePacket* pPacket);
	BOOL Broadcast(BasePacket* pPacket);

protected:
	BOOL OnConnectPlayer(int Serial);
	BOOL OnPlayerData(BasePacket* pPacket);
	BOOL OnDisconnectPlayer(int Serial);

private:
	ChatUserMap m_ChatUserMap;
};

