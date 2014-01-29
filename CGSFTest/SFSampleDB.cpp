#include "StdAfx.h"
#include "SFSampleDB.h"

SFSampleDB::SFSampleDB(void)
{
}

SFSampleDB::~SFSampleDB(void)
{
}

BOOL SFSampleDB::AddStatement()
{
	if (!RegisterStatement(m_Stmt_SPLoadUser))
		return FALSE;

	return TRUE;
}

BOOL SFSampleDB::SPLoadUser(int nUserID, TCHAR* szUserName)
{
	if(!IsDBConnected())
		return FALSE;

	m_Stmt_SPLoadUser.Init();

	SFQuery query(m_Stmt_SPLoadUser);

	m_Stmt_SPLoadUser.nUserID = nUserID;
	memcpy(m_Stmt_SPLoadUser.szUserName, szUserName, sizeof(TCHAR) * 10);

	if(!query.Execute())
	{
		return FALSE;
	}

	while (query.Fetch())
	{
	}

	return TRUE;
}
