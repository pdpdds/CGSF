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
	m_Dispatch.RegisterMessage(CGSF::Login, std::tr1::bind(&SFPlayerInit::OnLogin, this, std::tr1::placeholders::_1));
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

BOOL SFPlayerInit::ProcessPacket(BasePacket* pPacket)
{
	return m_Dispatch.HandleMessage(pPacket->GetPacketID(), pPacket);
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
BOOL SFPlayerInit::OnLogin(BasePacket* pPacket)
{
	SFProtobufPacket<SFPacketStore::Login>* pLogin = (SFProtobufPacket<SFPacketStore::Login>*)pPacket;

	SFPlayer* pPlayer = GetOwner();

	pPlayer->m_username = pLogin->GetData().username();
	pPlayer->m_password = pLogin->GetData().password();

	SFSendDBRequest::RequestLogin(pPlayer);

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