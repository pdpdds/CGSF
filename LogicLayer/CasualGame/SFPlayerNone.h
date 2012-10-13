#pragma once
#include "SFPlayerState.h"

class SFPlayerNone : public SFPlayerState
{
public:
	SFPlayerNone(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerNone(void);

protected:
	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;

private:
};
