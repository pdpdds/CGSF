#pragma once
#include "SFGameMode.h"

class SFFreeForAll : public SFGameMode
{
public:
	SFFreeForAll(int Mode);
	~SFFreeForAll(void);

	virtual SFGameMode* Clone()
	{
		return new SFFreeForAll(GetGameMode());
	}

	virtual BOOL OnEnter(int GameMode) override;
	virtual BOOL Onleave() override;

	virtual BOOL Update(DWORD dwTickcount) override;

	BOOL ProcessUserRequest(SFPlayer* pPlayer, SFPacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) override;

	BOOL OnSpawnPlayer(SFPlayer* pPlayer, SFPacket* pPacket);
	BOOL OnPlayerHealth(SFPlayer* pPlayer, SFPacket* pPacket);

protected:

private:
};
