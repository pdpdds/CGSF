#pragma once
#include <map>

class SFPlayer;

class SFLobby
{
	typedef std::map<int, SFPlayer*> PlayerMap;
public:
	SFLobby(void);
	virtual ~SFLobby(void);

	void OnEnter(SFPlayer* pPlayer);
	void OnLeave(SFPlayer* pPlayer);
	BOOL OnChat(SFPlayer* pPlayer, const std::string& szMessage);

protected:

private:
	PlayerMap m_PlayerMap;
};
