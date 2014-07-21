#include "stdafx.h"
#include "SFSessionService.h"
#include "EngineInterface/ISession.h"

bool SFSessionService::OnReceive(char* pData, unsigned short Length, _SessionDesc& desc)
{
	return m_pPacketProtocol->OnReceive(m_serial, pData, Length, desc);
}

/*bool SFSessionService::SendRequest(ISession* pSession, BasePacket* pPacket)
{
	if(TRUE == m_pPacketProtocol->SendRequest(pSession, pPacket))
	{
		return true;
	}

	return false;
}*/