#pragma once
#include "SFDispatch.h"

class SFRoom;
class SFPlayer;
class SFPacket;

class SFRoomState
{
	friend class SFRoomFSM;
public:
	SFRoomState(SFRoom* pOwner, eRoomState State);
	virtual ~SFRoomState(void);

	eRoomState GetRoomState(){return m_State;}
	SFRoom* GetOwner(){return m_pOwner;}

protected:
	virtual BOOL OnEnter(){return FALSE;}
	virtual BOOL OnLeave(){return FALSE;}

	virtual BOOL Update(DWORD dwTickCount){return FALSE;}

	virtual BOOL OnEnterRoom(SFPlayer* pPlayer){return FALSE;}
	virtual BOOL OnLeaveRoom(SFPlayer* pPlayer){return FALSE;}


	virtual BOOL ProcessUserRequest(SFPlayer* pPlayer, BasePacket* pPacket){return FALSE;}
	virtual BOOL ProcessUserRequest(SFPlayer* pPlayer, int Msg) {return FALSE;}

	SFDispatch<USHORT, std::tr1::function<BOOL(SFPlayer*)>, SFPlayer*> m_Dispatch;
	SFDispatch_2<USHORT, std::tr1::function<BOOL(SFPlayer*, BasePacket*)>, SFPlayer*, BasePacket*> m_DispatchingSystem;

private:
	SFRoom* m_pOwner;
	eRoomState m_State;
};
