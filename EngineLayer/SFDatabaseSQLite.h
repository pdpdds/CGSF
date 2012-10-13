#pragma once
#include "SFDatabase.h"

class CppSQLite3DB;

class SFDatabaseSQLite : public SFDatabase
{
public:
	SFDatabaseSQLite(void);
	virtual ~SFDatabaseSQLite(void);

	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;

	BOOL Execute(char* szQuery);

protected:

private:
	//CppSQLite3DB* m_pDatabase;
};