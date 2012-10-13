#pragma once
#include "SFPlayerState.h"

class SFPlayerPlayEnd : public SFPlayerState
{
public:
	SFPlayerPlayEnd(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerPlayEnd(void);
};
