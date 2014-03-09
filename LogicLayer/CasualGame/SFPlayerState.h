#pragma once
#include "SFDispatch.h"
#include "SFGameStructure.h"

class SFPlayer;
class BasePacket;
class SFMessage;

class SFPlayerState
{
	friend class SFPlayerFSM;
public:
	SFPlayerState(SFPlayer* pOwner, ePlayerState State);
	virtual ~SFPlayerState(void);

	ePlayerState GetPlayerState(){return m_State;}
	SFPlayer* GetOwner(){return m_pOwner;}

protected:
	virtual BOOL OnEnter(){return FALSE;}
	virtual BOOL OnLeave(){return FALSE;}
	virtual BOOL ProcessPacket(BasePacket* pPacket){return FALSE;}
	virtual BOOL ProcessDBResult(SFMessage* pMessage){return FALSE;}

	SFDispatch<USHORT, std::tr1::function<BOOL(BasePacket*)>, BasePacket*> m_Dispatch;

private:
	SFPlayer* m_pOwner;
	ePlayerState m_State;
};