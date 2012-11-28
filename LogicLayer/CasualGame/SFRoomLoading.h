#pragma once
#include "SFRoomState.h"

class SFRoom;

class SFRoomLoading : public SFRoomState
{
public:
	SFRoomLoading(SFRoom* pOwner, eRoomState State);
	~SFRoomLoading(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;

	virtual BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;

protected:

	BOOL OnLoadingComplete(SFPlayer* pPlayer, BasePacket* pPacket);

};
