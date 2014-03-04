/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#ifndef SQLSVRPOOL_H
#define SQLSVRPOOL_H
#include <windows.h>
#include <iostream>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>


#include "..\CommonDatabasePool.cpp"

using namespace std;


class SQLSvrPool : public CommonDatabasePool<SQLHANDLE>{
public:
	SQLHANDLE* MyGetConcreteConnection();
	void MyReleaseConcreteConnection(SQLHANDLE*);
	int MyPingConcreteConnection(SQLHANDLE*);
	int MyGetPingTimeout();
	SQLSvrPool(string, string, string, string, unsigned int timeout=300000, string query="select 0 from dual");

	void ShowSQLError(ostream&, const unsigned int, const SQLHANDLE&);

private:
	SQLHANDLE sqlenvhandle;
	string username;
	string password;
	string database;
	string host;
	string keepalivequery;
	unsigned int keepalivetimeout;
};

#endif
