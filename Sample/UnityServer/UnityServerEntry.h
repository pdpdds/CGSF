#pragma once
#include "ILogicEntry.h"
#include "LoginPart.h"
class GameUser;
class BasePacket;

class UnityServerEntry :public ILogicEntry
{
	typedef std::map<int, GameUser*> DicUserMap;
public:
	UnityServerEntry();
	virtual ~UnityServerEntry();

	virtual bool Initialize() override;
	virtual bool ProcessPacket(BasePacket* pBasePacket) override;
	bool Broadcast(BasePacket* pPacket);
	GameUser* FindUser(int serial);

protected:
	bool OnConnectPlayer(int serial);
	bool OnPlayerData(BasePacket* pPacket);
	bool OnDisconnectPlayer(int serial);

private:
	DicUserMap m_ChatUserMap;
	LoginPart m_Login;

};