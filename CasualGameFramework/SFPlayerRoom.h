#pragma once
#include "SFPlayerState.h"

class SFPlayerRoom : public SFPlayerState
{
public:
	SFPlayerRoom(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerRoom(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;
	virtual BOOL ProcessPacket(SFPacket* pPacket) override;
	virtual BOOL ProcessDBResult(SFMessage* pMessage) override;
};
