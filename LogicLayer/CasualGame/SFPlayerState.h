#pragma once

class SFPlayer;
class SFPacket;
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
	virtual BOOL ProcessPacket(SFPacket* pPacket){return FALSE;}
	virtual BOOL ProcessDBResult(SFMessage* pMessage){return FALSE;}

private:
	SFPlayer* m_pOwner;
	ePlayerState m_State;
};