#pragma once
#include "SFRoomState.h"

class SFRoom;

class SFRoomWait : public SFRoomState
{
public:
	SFRoomWait(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomWait(void);

	BOOL ProcessUserRequest(SFPlayer* pPlayer, SFPacket* pPacket) override;
		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
	BOOL OnChangeTeam(SFPlayer* pPlayer, SFPacket* pPacket );
	BOOL OnStartGame(SFPlayer* pPlayer, SFPacket* pPacket );
	BOOL OnChat( SFPlayer* pPlayer, SFPacket* pPacket );

protected:
	BOOL OnEnterRoom(SFPlayer* pPlayer) override;
	BOOL OnLeaveRoom(SFPlayer* pPlayer) override;
};
