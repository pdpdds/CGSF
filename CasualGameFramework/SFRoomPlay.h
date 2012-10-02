#pragma once
#include "SFRoomState.h"

class SFRoom;
class SFGameModeFSM;

class SFRoomPlay : public SFRoomState
{
public:
	SFRoomPlay(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomPlay(void);

	virtual BOOL OnEnter() override;
	virtual BOOL OnLeave() override;

	BOOL Update(DWORD dwTickCount) override;

	BOOL ProcessUserRequest(SFPlayer* pPlayer, SFPacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) override;

	BOOL OnEnterRoom(SFPlayer* pPlayer) override;
	BOOL OnLeaveRoom( SFPlayer* pPlayer)  override;

	BOOL OnLoadingComplete( SFPlayer* pPlayer, SFPacket* pPacket );

protected:

private:
	SFGameModeFSM* m_pGameModeFSM;
	BOOL m_bGameEnd;
};
