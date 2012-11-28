#pragma once
#include "SFPlayerState.h"

class SFPlayerLobby : public SFPlayerState
{
public:
	SFPlayerLobby(SFPlayer* pOwner, ePlayerState State);
	~SFPlayerLobby(void);

	BOOL OnEnterRoom(BasePacket* pPacket);
	BOOL OnCreateRoom(BasePacket* pPacket);
	BOOL OnChatReq(BasePacket* pPacket);
	BOOL OnPlayerIP( BasePacket* pPacket );

protected:
	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;
	virtual BOOL ProcessPacket(BasePacket* pPacket) override;
	virtual BOOL ProcessDBResult(SFMessage* pMessage) override;

private:
};
