#pragma once
#include "SFPlayerState.h"

class SFPlayerLogin : public SFPlayerState
{
public:
	SFPlayerLogin(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerLogin(void);

	BOOL OnEnterLobby(BasePacket* pPacket);

protected:
	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;
	virtual BOOL ProcessPacket(BasePacket* pPacket) override;
	virtual BOOL ProcessDBResult(SFMessage* pMessage) override;
};
