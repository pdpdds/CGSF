#pragma once
#include "SFDatabase.h"
#include "SFDispatch.h"
#include <functional>
#include "Macro.h"
#include "BasePacket.h"

#pragma warning(disable : 4005)
#include <my_global.h>
#include <mysql.h>

class BasePacket;

class SFDatabaseMySQL : public SFDatabase
{
public:
	SFDatabaseMySQL(void)
	{
		m_pDBConnection = NULL;
	}

	virtual ~SFDatabaseMySQL(void)
	{
		if(m_pDBConnection)
			mysql_close(m_pDBConnection);
	}

	virtual BOOL Initialize()
	{
		SFASSERT(mysql_thread_safe() > 0);

		//LOG(ERROR) << "MySQL Version : " <<  mysql_get_client_info();

		mysql_init(&m_Conn);

		m_pDBConnection = mysql_real_connect(&m_Conn, 
			GetInfo()->IP,	
			GetInfo()->szID, 
			GetInfo()->szPassword, 
			GetInfo()->szDBName, 
			GetInfo()->Port,
			(char *)NULL, 
			0);

		if(m_pDBConnection==NULL)
		{
			//LOG(FATAL) << "Mysql Connection error";
			SFASSERT(0);
			return FALSE;
		}

		mysql_query(m_pDBConnection,"set session character_set_connection=euckr;");
		mysql_query(m_pDBConnection,"set session character_set_results=euckr;");
		mysql_query(m_pDBConnection,"set session character_set_client=euckr;");	

		return TRUE;
	}

	BOOL Execute(char* szQuery)
	{
		int QueryState;

		QueryState = mysql_query(m_pDBConnection, szQuery);

		if (QueryState != 0)
		{
			//LOG(FATAL) << "Mysql Query error" << mysql_error(&m_Conn);
			SFASSERT(0);
			return FALSE;
		}

		return TRUE;
	}

	MYSQL* GetDBConnection(){return m_pDBConnection;}

	virtual BOOL RegisterDBService() = 0;

protected:
	SFDispatch<USHORT, std::tr1::function<BOOL(BasePacket*)>, BasePacket*> m_Dispatch;

private:
	MYSQL m_Conn;
	MYSQL* m_pDBConnection;
};