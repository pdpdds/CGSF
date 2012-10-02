#pragma once
#include "SFPlayerState.h"

class SFPlayerLobby : public SFPlayerState
{
public:
	SFPlayerLobby(SFPlayer* pOwner, ePlayerState State);
	~SFPlayerLobby(void);

	BOOL OnEnterRoom(SFPacket* pPacket);
	BOOL OnCreateRoom(SFPacket* pPacket);
	BOOL OnChatReq(SFPacket* pPacket);
	BOOL OnPlayerIP( SFPacket* pPacket );

protected:
	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;
	virtual BOOL ProcessPacket(SFPacket* pPacket) override;
	virtual BOOL ProcessDBResult(SFMessage* pMessage) override;

private:
};
