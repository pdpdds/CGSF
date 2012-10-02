#include "StdAfx.h"
#include "SFDBRequest.h"
#include "SFDatabaseMySQL.h"

void SendToLogic(SFMessage* pMessage)
{
	pMessage->SetPacketType(SFCommand_DB);
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

	LogicEntrySingleton::instance()->RecallDBMessage(m_pMessage);
	m_pMessage = NULL;
	
	return 0;
}