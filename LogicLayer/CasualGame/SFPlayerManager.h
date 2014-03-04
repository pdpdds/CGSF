#pragma once
class SFPlayerManager
{
	typedef std::map<std::string, SFPlayer*> PlayerMap;

public:
	SFPlayerManager(void);
	virtual ~SFPlayerManager(void);

	bool AddPlayer(SFPlayer* pPlayer);
	void DelPlayer(SFPlayer* pPlayer);

private:
	PlayerMap m_PlayerMap;
};

