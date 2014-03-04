#pragma once
#include "SFDatabase.h"

class MySQLPool;

class SFMySQL
{
public:
	SFMySQL(void)
	{
		m_pMysqlPool = NULL;
	}

	virtual ~SFMySQL(void);

	BOOL Initialize(_DBConnectionInfo* pInfo);
	BOOL Execute(char* szQuery);

	MySQLPool* GetMySqlPool(){return m_pMysqlPool;}

protected:

private:
	MySQLPool* m_pMysqlPool;
};