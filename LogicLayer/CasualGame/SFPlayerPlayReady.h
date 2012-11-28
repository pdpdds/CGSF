#pragma once
#include "SFPlayerState.h"

class SFPlayerPlayReady : public SFPlayerState
{
public:
	SFPlayerPlayReady(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerPlayReady(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;
	virtual BOOL ProcessPacket(BasePacket* pPacket) override;
	virtual BOOL ProcessDBResult(SFMessage* pMessage) override;
};
