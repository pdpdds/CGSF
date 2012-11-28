#include "StdAfx.h"
#include "SFDBRequest.h"
#include "SFDatabaseMySQL.h"

SFMessage* GetDBInitMessage(int RequestMsg, DWORD PlayerSerial)
{
	SFMessage* pMessage = LogicEntrySingleton::instance()->AllocDBMessage();

	SFASSERT(pMessage != NULL);
	pMessage->Initialize(RequestMsg);
	pMessage->SetOwnerSerial(PlayerSerial);
	pMessage->SetPacketType(SFPacket_DB);

	return pMessage;
}

void SendToLogic(SFMessage* pMessage)
{
	pMessage->SetPacketType(SFPacket_DB);
	LogicGatewaySingleton::instance()->PushPacket(pMessage);
}

extern ACE_TSS<SFDatabaseMySQL> Database;

SFDBRequest::SFDBRequest(void)
{
}

SFDBRequest::~SFDBRequest(void)
{
}

int SFDBRequest::call(void)
{
	SFASSERT(m_pMessage != NULL);

	Database->Call(m_pMessage);

	LogicEntrySingleton::instance()->ReleaseDBMessage(m_pMessage);
	m_pMessage = NULL;
	
	return 0;
}