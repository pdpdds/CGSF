#include "StdAfx.h"
#include "LogicEntry.h"

LogicEntry::LogicEntry(void)
: m_pLogicEntry(NULL)
{
}

LogicEntry::~LogicEntry(void)
{
}

BOOL LogicEntry::ProcessPacket( BasePacket* pPacket )
{
	if(m_pLogicEntry)
		m_pLogicEntry->ProcessPacket(pPacket);

	return TRUE;
}