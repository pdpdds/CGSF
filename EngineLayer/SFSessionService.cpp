#include "stdafx.h"
#include "SFSessionService.h"

bool SFSessionService::OnReceive(char* pData, unsigned short Length, bool bServerObject)
{
	return m_pPacketProtocol->OnReceive(m_Serial, pData, Length, bServerObject);
}

/*bool SFSessionService::SendRequest(ISession* pSession, BasePacket* pPacket)
{
	if(TRUE == m_pPacketProtocol->SendRequest(pSession, pPacket))
	{
		return true;
	}

	return false;
}*/