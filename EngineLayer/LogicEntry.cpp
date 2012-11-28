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

BOOL LogicEntry::ProcessPacket( BasePacket* pPacket )
{
	if(m_pLogicEntry)
		m_pLogicEntry->ProcessPacket(pPacket);

	return TRUE;
}

SFMessage* LogicEntry::AllocDBMessage()
{
	return m_DBMessagePool.Alloc();
}

BOOL LogicEntry::ReleaseDBMessage( SFMessage* pMessage )
{
	return m_DBMessagePool.Release(pMessage);
}