#include "stdafx.h"
#include "SFSendDBRequest.h"
#include "SFPlayer.h"
#include "DBMsg.h"

///////////////////////////////////////////////////////////////////////////////////////
//Internal Function
///////////////////////////////////////////////////////////////////////////////////////


BOOL SFSendDBRequest::SendDBRequest(SFMessage* pMessage)
{
	return SFLogicEntry::GetLogicEntry()->GetDataBaseProxy()->SendDBRequest(pMessage);
}

///////////////////////////////////////////////////////////////////////////////////////
//Default DB Request
///////////////////////////////////////////////////////////////////////////////////////
BOOL SFSendDBRequest::SendDBRequest(int RequestMsg, DWORD PlayerSerial, BasePacket* pPacket)
{
	SFPacket* Packet = (SFPacket*)pPacket;

	SFMessage* pMessage = SFDatabase::GetInitMessage(RequestMsg, PlayerSerial);
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
	SFMessage* pMessage = SFDatabase::GetInitMessage(DBMSG_LOGIN, pPlayer->GetSerial());
	pMessage->Write((const BYTE* )pPlayer->m_username.c_str(), pPlayer->m_username.length());
	pMessage->Write((const BYTE* )pPlayer->m_password.c_str(), pPlayer->m_password.length());

	return SendDBRequest(pMessage);
}