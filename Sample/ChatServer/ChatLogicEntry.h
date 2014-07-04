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

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
	bool Broadcast(BasePacket* pPacket);
	ChatUser* FindUser(int serial);

protected:
	bool OnConnectPlayer(int serial);
	bool OnPlayerData(BasePacket* pPacket);
	bool OnDisconnectPlayer(int serial);

private:
	ChatUserMap m_ChatUserMap;
};

