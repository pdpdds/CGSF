#include "stdafx.h"
#include "SFSendDBRequest.h"
#include "SFPlayer.h"
#include "DBMsg.h"

///////////////////////////////////////////////////////////////////////////////////////
//Internal Function
///////////////////////////////////////////////////////////////////////////////////////
SFMessage* SFSendDBRequest::GetInitMessage(int RequestMsg, DWORD PlayerSerial)
{
	SFMessage* pMessage = LogicEntrySingleton::instance()->AllocDBMessage();

	SFASSERT(pMessage != NULL);
	pMessage->Initialize(RequestMsg);
	pMessage->SetPacketID(RequestMsg);
	pMessage->SetOwnerSerial(PlayerSerial);
	pMessage->SetPacketType(SFPacket_DB);

	return pMessage;
}

BOOL SFSendDBRequest::Send(SFMessage* pMessage)
{
	return SFLogicEntry::GetLogicEntry()->GetDataBaseProxy()->SendDBRequest(pMessage);
}

///////////////////////////////////////////////////////////////////////////////////////
//Default DB Request
///////////////////////////////////////////////////////////////////////////////////////
BOOL SFSendDBRequest::SendRequest(int RequestMsg, DWORD PlayerSerial, BasePacket* pPacket)
{
	SFPacket* Packet = (SFPacket*)pPacket;

	SFMessage* pMessage = GetInitMessage(RequestMsg, PlayerSerial);
	pMessage->Write(Packet->GetDataBuffer(), Packet->GetDataSize());
	SFLogicEntry::GetLogicEntry()->GetDataBaseProxy()->SendDBRequest(pMessage);

	return TRUE;
}

void SFSendDBRequest::SendToLogic(BasePacket* pMessage)
{
	pMessage->SetPacketType(SFPacket_DB);
	LogicGatewaySingleton::instance()->PushPacket(pMessage);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//개별적으로 리퀘스트를 작성해야 할 때
//////////////////////////////////////////////////////////////////////////////////////////////
BOOL SFSendDBRequest::RequestLogin(SFPlayer* pPlayer)
{
	SFMessage* pMessage = GetInitMessage(DBMSG_LOGIN, pPlayer->GetSerial());
	pMessage->Write((const BYTE* )pPlayer->m_username.c_str(), pPlayer->m_username.length());
	pMessage->Write((const BYTE* )pPlayer->m_password.c_str(), pPlayer->m_password.length());

	return Send(pMessage);
}