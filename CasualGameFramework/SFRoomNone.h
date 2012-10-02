#pragma once
#include "SFRoomState.h"

class SFRoom;
class SFGameModeFSM;

class SFRoomNone : public SFRoomState
{
public:
	SFRoomNone(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomNone(void);
};
