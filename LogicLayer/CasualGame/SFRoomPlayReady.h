#pragma once
#include "SFRoomState.h"

class SFRoom;

class SFRoomPlayReady : public SFRoomState
{
public:
	SFRoomPlayReady(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomPlayReady(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;

	virtual BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;

protected:
	BOOL OnReportIP(SFPlayer* pPlayer, BasePacket* pPacket);
};
