#include "StdAfx.h"
#include "SFPlayerInit.h"
#include "SFPlayer.h"
#include "SFDataBaseProxy.h"
#include "SFPacketStore.pb.h"
#include "PacketID.h"
#include "DBMsg.h"
#include "SFSendPacket.h"
#include "SFSendDBRequest.h"


SFPlayerInit::SFPlayerInit(SFPlayer* pOwner, ePlayerState State)
: SFPlayerState(pOwner, State)
{
}

SFPlayerInit::~SFPlayerInit(void)
{
}

BOOL SFPlayerInit::OnEnter()
{
	return TRUE;
}

BOOL SFPlayerInit::OnLeave()
{
	return TRUE;
}

BOOL SFPlayerInit::ProcessPacket(SFPacket* pPacket)
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::Login:
		{
			OnLogin(pPacket);
		}
		break;

	default:
		return FALSE;
	}
	
	return TRUE;
}

BOOL SFPlayerInit::ProcessDBResult(SFMessage* pMessage)
{
	switch(pMessage->GetCommand())
	{
	case DBMSG_LOGIN:
		{
			OnDBLogin(pMessage);
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Handle Packet Recv
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFPlayerInit::OnLogin(SFPacket* pPacket)
{
	SFPacketStore::Login PktLogin;
	protobuf::io::ArrayInputStream is(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	PktLogin.ParseFromZeroCopyStream(&is);

	SFPlayer* pPlayer = GetOwner();

	pPlayer->m_username = PktLogin.username();
	pPlayer->m_password = PktLogin.password();

	SFSendDBRequest::RequestLogin(pPlayer, pPacket);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Handle DB Result
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFPlayerInit::OnDBLogin(SFMessage* pMessage)
{
	SFPlayer* pPlayer = GetOwner();

	int Result = 0;

	*pMessage >> Result;

	SendLoginResult(pPlayer, Result);

	if(DBRESULT_STATE_SUCCESS == Result)
	{
		pPlayer->ChangeState(PLAYER_STATE_LOGIN);
	}
	else
	{
		//pPlayer->ChangeState(PLAYER_STATE_NONE);
	}

	return TRUE;
}