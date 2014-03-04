#pragma once
#include "SFRoomState.h"

class SFRoom;

class SFRoomPlayEnd : public SFRoomState
{
public:
	SFRoomPlayEnd(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomPlayEnd(void);
};
