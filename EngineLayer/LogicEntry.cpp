#include "StdAfx.h"
#include "LogicEntry.h"

LogicEntry::LogicEntry(void)
: m_pLogicEntry(NULL)
{
}

LogicEntry::~LogicEntry(void)
{
}

bool LogicEntry::ProcessPacket(BasePacket* pPacket)
{
	if (m_pLogicEntry)
	{		
		_SessionDesc& desc = pPacket->GetSessionDesc();
		if (desc.sessionType <= 0)
			m_pLogicEntry->ProcessPacket(pPacket);
		else
		{
			m_pLogicEntry->ProcessConnectorPacket(pPacket);
		}
	}
		
	return true;
}