#include "StdAfx.h"
#include "SFPlayerNone.h"

SFPlayerNone::SFPlayerNone(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerNone::~SFPlayerNone(void)
{
}

BOOL SFPlayerNone::OnEnter()
{
	return TRUE;
}

BOOL SFPlayerNone::OnLeave()
{
	return TRUE;
}
