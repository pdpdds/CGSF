#include "StdAfx.h"
#include "SFDBManager.h"

SFDBManager::SFDBManager(void)
: m_ShutDown(0)
, m_Workers_Lock()
, m_Workers_Cond(m_Workers_Lock)
{
}

SFDBManager::~SFDBManager(void)
{
}
