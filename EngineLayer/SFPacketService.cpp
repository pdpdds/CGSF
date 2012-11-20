#include "stdafx.h"
#include "SFPacketService.h"

SFPacketService::SFPacketService(void)
{
}


SFPacketService::~SFPacketService(void)
{
}

bool SFPacketService::OnData(int Serial, char* pData, unsigned short Length)
{
	return m_UserSession.ProcessData(Serial, pData, Length);
}
