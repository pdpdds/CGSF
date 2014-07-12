#pragma once
#include "SFGameMode.h"

class SGTraining : public SFGameMode
{
public:
	SGTraining(int mode);
	virtual ~SGTraining(void);

	virtual SFGameMode* Clone()
	{
		return new SGTraining(GetGameMode());
	}

	virtual BOOL OnEnter(int gameMode) override;
	virtual BOOL Onleave() override;

	virtual BOOL Update(DWORD dwTickcount) override;

	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int msg) override;
};
