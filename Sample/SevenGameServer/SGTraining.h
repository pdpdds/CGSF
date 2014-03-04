#pragma once
#include "SFGameMode.h"

class SGTraining : public SFGameMode
{
public:
	SGTraining(int Mode);
	virtual ~SGTraining(void);

	virtual SFGameMode* Clone()
	{
		return new SGTraining(GetGameMode());
	}

	virtual BOOL OnEnter(int GameMode) override;
	virtual BOOL Onleave() override;

	virtual BOOL Update(DWORD dwTickcount) override;

	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) override;
};
