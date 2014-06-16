#include "stdafx.h"
#include "SFDatabase.h"

SFObjectPool<SFMessage> SFDatabase::m_DBMessagePool;
_DBConnectionInfo SFDatabase::m_Info;

SFDatabase::SFDatabase(SFDBAdaptor* pAdaptor)
	: m_pAdaptor(pAdaptor)
{
	//m_DBMessagePool.Init(1000);
}

SFMessage* SFDatabase::GetInitMessage(int RequestMsg, DWORD PlayerSerial)
{
	SFMessage* pMessage = AllocDBMsg();

	SFASSERT(pMessage != NULL);
	pMessage->Initialize(RequestMsg);
	pMessage->SetPacketID(RequestMsg);
	pMessage->SetSerial(PlayerSerial);
	pMessage->SetPacketType(SFPACKET_DB);

	return pMessage;
}

SFMessage* SFDatabase::AllocDBMsg()
{
	return m_DBMessagePool.Alloc();
}

BOOL SFDatabase::RecallDBMsg( SFMessage* pMessage )
{
	return m_DBMessagePool.Release(pMessage);
}

BOOL SFDatabase::Call(BasePacket* pMessage)
{
	ASSERT(pMessage->GetPacketType() == SFPACKET_DB);
	m_pAdaptor->Call(pMessage);
	RecallDBMsg((SFMessage*)pMessage);

	return TRUE;
}