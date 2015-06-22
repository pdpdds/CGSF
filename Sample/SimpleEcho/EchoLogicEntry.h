#pragma once
#include "ILogicEntry.h"
#include <map>

class BasePacket;
class GameUser;

class EchoLogicEntry : public ILogicEntry
{
	typedef std::map<int, GameUser*> GameUserMap;
public:
	EchoLogicEntry(void);
	virtual ~EchoLogicEntry(void);	

	bool OnConnect(BasePacket* pPacket);
	bool OnDisconnect(BasePacket* pPacket);
	bool Broadcast(BasePacket* pPacket);
	GameUser* FindUser(int serial);

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;

private:
	GameUserMap m_GameUserMap;
};

