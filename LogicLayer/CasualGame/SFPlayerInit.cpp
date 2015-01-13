#include "StdAfx.h"
#include "SFPlayerInit.h"
#include "SFPlayer.h"
#include "SFDataBaseProxy.h"
#include "DBMsg.h"
#include "SFSendPacket.h"
#include "SFPlayerManager.h"


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

	SFMessage* pMessage = SFDBPacketSystem<SFMessage>::GetInstance()->GetInitMessage(DBMSG_LOGIN, pPlayer->GetSerial());

	*pMessage << (char*)pPlayer->m_username.c_str();
	*pMessage << (char*)pPlayer->m_password.c_str();

	return SFDBPacketSystem<SFMessage>::GetInstance()->SendDBRequest(pMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Handle DB Result
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFPlayerInit::OnDBLogin(SFMessage* pMessage)
{
	SFPlayer* pPlayer = GetOwner();

	int Result = 0;

	*pMessage >> Result;

	if(DBRESULT_STATE_SUCCESS == Result)
	{
		if(TRUE == SFLogicEntry::GetLogicEntry()->GetPlayerManager()->AddPlayer(pPlayer))
			pPlayer->ChangeState(PLAYER_STATE_LOGIN);
		else
			Result = -1;
	}
	else
	{
		//pPlayer->ChangeState(PLAYER_STATE_NONE);
	}

	SendLoginResult(pPlayer, Result);

	return TRUE;
}