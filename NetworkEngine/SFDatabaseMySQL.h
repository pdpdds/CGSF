#pragma once
#include "SFDatabase.h"

#pragma warning(disable : 4005)
#include <my_global.h>
#include <mysql.h>

class SFDatabaseMySQL : public SFDatabase
{
public:
	SFDatabaseMySQL(void);
	virtual ~SFDatabaseMySQL(void);

	virtual BOOL Initialize(_DBConnectionInfo* pInfo) override;
	virtual BOOL Call(SFMessage* pMessage) override;

	BOOL Execute(char* szQuery);
	MYSQL* GetDBConnection(){return m_pDBConnection;}

protected:

private:
	MYSQL m_Conn;
	MYSQL* m_pDBConnection;
	
};
