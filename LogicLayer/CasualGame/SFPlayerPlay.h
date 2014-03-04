#pragma once
#include "SFPlayerState.h"

class SFPlayerPlay : public SFPlayerState
{
public:
	SFPlayerPlay(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerPlay(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;
	virtual BOOL ProcessPacket(BasePacket* pPacket) override;

protected:
};
