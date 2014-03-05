#include "StdAfx.h"
#include "SFPlayerLogin.h"
#include "SFPlayer.h"
#include "SFDataBaseProxy.h"
#include "DBMsg.h"

SFPlayerLogin::SFPlayerLogin(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
	m_Dispatch.RegisterMessage(CGSF::EnterLobby, std::tr1::bind(&SFPlayerLogin::OnEnterLobby, this, std::tr1::placeholders::_1));
}

SFPlayerLogin::~SFPlayerLogin(void)
{
}

BOOL SFPlayerLogin::OnEnter()
{
	return TRUE;
}

BOOL SFPlayerLogin::OnLeave()
{
	return TRUE;
}

BOOL SFPlayerLogin::ProcessPacket( BasePacket* pPacket )
{
	return m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPacket);

	return TRUE;
}

BOOL SFPlayerLogin::ProcessDBResult( SFMessage* pMessage )
{
	return FALSE;
}

BOOL SFPlayerLogin::OnEnterLobby( BasePacket* pPacket )
{
	SFProtobufPacket<SFPacketStore::EnterLobby>* pEnterLobby = (SFProtobufPacket<SFPacketStore::EnterLobby>*)pPacket;

	SFPlayer* pPlayer = GetOwner();

	pPlayer->ChangeState(PLAYER_STATE_LOBBY);

	return TRUE;
}