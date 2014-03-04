#pragma once
#include <map>
#include "SFPlayerState.h"

class SFPlayerFSM : public SFPlayerState
{
	friend class SFPlayer;

	typedef std::map<ePlayerState, SFPlayerState*> PlayerStateMap;
public:
	SFPlayerFSM(SFPlayer* pOwner, ePlayerState State);
	~SFPlayerFSM(void);

	BOOL Reset();

protected:
	BOOL ChangeState(ePlayerState State);
	BOOL AddState(ePlayerState State);
	ePlayerState GetPlayerState();
	BOOL ProcessPacket(BasePacket* pPacket);
	BOOL ProcessDBResult(SFMessage* pMessage);

private:
	PlayerStateMap m_PlayerStateMap;
	SFPlayerState* m_pCurrentState;
};
