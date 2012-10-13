#include "StdAfx.h"
#include "SFPlayerState.h"

SFPlayerState::SFPlayerState(SFPlayer* pOwner, ePlayerState State)
: m_pOwner(pOwner)
, m_State(State)
{
}

SFPlayerState::~SFPlayerState(void)
{
}
