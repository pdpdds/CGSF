#include "StdAfx.h"
#include "SFRoomState.h"

SFRoomState::SFRoomState( SFRoom* pOwner, eRoomState State )
: m_pOwner(pOwner)
, m_State(State)
{
}

SFRoomState::~SFRoomState(void)
{
}
