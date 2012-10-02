#include "StdAfx.h"
#include "LogicEntry.h"

LogicEntry::LogicEntry(void)
: m_pLogicEntry(NULL)
{
	m_DBMessagePool.Init(1000);
}

LogicEntry::~LogicEntry(void)
{
}

BOOL LogicEntry::ProcessPacket( SFCommand* pCommand )
{
	if(m_pLogicEntry)
		m_pLogicEntry->ProcessPacket(pCommand);

	return TRUE;
}

SFMessage* LogicEntry::GetDBMessage()
{
	return m_DBMessagePool.Alloc();
}

BOOL LogicEntry::RecallDBMessage( SFMessage* pMessage )
{
	return m_DBMessagePool.Release(pMessage);
}