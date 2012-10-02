#include "stdafx.h"
#include "SFSendDBRequest.h"
#include "SFPlayer.h"
#include "DBMsg.h"

///////////////////////////////////////////////////////////////////////////////////////
//Default DB Request
///////////////////////////////////////////////////////////////////////////////////////
BOOL SFSendDBRequest::SendRequest(int RequestMsg, DWORD PlayerSerial, SFPacket* pPacket)
{
	SFMessage* pMessage = GetInitMessage(RequestMsg, PlayerSerial);
	pMessage->Write(pPacket->GetDataBuffer(), pPacket->GetDataSize());
	SFLogicEntry::GetLogicEntry()->GetDataBaseProxy()->SendDBRequest(pMessage);

	return TRUE;
}

SFMessage* SFSendDBRequest::GetInitMessage(int RequestMsg, DWORD PlayerSerial)
{
	SFMessage* pMessage = LogicEntrySingleton::instance()->GetDBMessage();

	SFASSERT(pMessage != NULL);
	pMessage->Initialize(DBMSG_LOGIN);
	pMessage->SetOwnerSerial(PlayerSerial);
	pMessage->SetPacketType(SFCommand_DB);

	return pMessage;
}

BOOL SFSendDBRequest::RequestLogin(SFPlayer* pPlayer, SFPacket* pPacket)
{
	SFMessage* pMessage = GetInitMessage(DBMSG_LOGIN, pPlayer->GetSerial());
	pMessage->Write(pPacket->GetDataBuffer(), pPacket->GetDataSize());

	SFLogicEntry::GetLogicEntry()->GetDataBaseProxy()->SendDBRequest(pMessage);

	return TRUE;
}