#pragma once
#include "SFMSSQL.h"

class SFStmt_SPLoadUser : public SFStatement
{
public:
	int nErrorCode;
	int nUserID;
	TCHAR szUserName[10];

	void Init(){nErrorCode = nUserID = 0, memset(szUserName, 0, sizeof(TCHAR) * 10);}

protected:
	DEFINE_QUERY(L"{call SPLoadUser(?)}")

	START_BIND_PARAM()
		BIND_OUTPUT_PARAM(nErrorCode)
	END_BIND()

	START_BIND_COLUMN()
		BIND_COLUMN(nUserID)
		BIND_COLUMN(szUserName)
	END_BIND()
};


class SFSampleDB : public SFMSSQL
{
public:
	SFSampleDB(void);
	virtual ~SFSampleDB(void);

	BOOL AddStatement();

	BOOL SPLoadUser(int nUserID, TCHAR* szUserName);

protected:

private:
	SFStmt_SPLoadUser m_Stmt_SPLoadUser;
};
