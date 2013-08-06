#pragma once
#include "SFDatabase.h"
#include <functional>
#include "Macro.h"
#include "BasePacket.h"

#pragma warning(disable : 4005)
#include <my_global.h>
#include <mysql.h>

class BasePacket;

class SFMySQL
{
public:
	SFMySQL(void)
	{
		m_pDBConnection = NULL;
	}

	virtual ~SFMySQL(void)
	{
		if(m_pDBConnection)
			mysql_close(m_pDBConnection);
	}

	BOOL Initialize(_DBConnectionInfo* pInfo);
	BOOL Execute(char* szQuery);

	MYSQL* GetDBConnection(){return m_pDBConnection;}

protected:

private:
	MYSQL m_Conn;
	MYSQL* m_pDBConnection;
};