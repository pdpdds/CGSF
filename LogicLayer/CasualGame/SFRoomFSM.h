#pragma once
#include "SFRoomState.h"

class SFRoomFSM : public SFRoomState
{
	friend class SFRoom;

	typedef std::map<eRoomState, SFRoomState*> RoomStateMap;

public:
	SFRoomFSM(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomFSM(void);

	virtual BOOL Update(DWORD dwTickCount) override;
	BOOL Reset();
	eRoomState GetRoomState(){return m_pCurrentState->GetRoomState();}

	BOOL OnEnterRoom(SFPlayer* pPlayer) override;
	BOOL OnLeaveRoom( SFPlayer* pPlayer)  override;

protected:
	BOOL AddState(eRoomState State);
	BOOL ChangeState(eRoomState State);
	BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket) override;
	BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) override;

private:
	RoomStateMap m_RoomStateMap;
	SFRoomState* m_pCurrentState;
};