#pragma once
#include "SFRoomState.h"

class SFRoom;

class SFRoomWait : public SFRoomState
{
public:
	SFRoomWait(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomWait(void);

	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;
		
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
	BOOL OnChangeTeam(SFPlayer* pPlayer, BasePacket* pPacket );
	BOOL OnStartGame(SFPlayer* pPlayer, BasePacket* pPacket );
	BOOL OnChat( SFPlayer* pPlayer, BasePacket* pPacket );

protected:
	BOOL OnEnterRoom(SFPlayer* pPlayer) override;
	BOOL OnLeaveRoom(SFPlayer* pPlayer) override;
};
