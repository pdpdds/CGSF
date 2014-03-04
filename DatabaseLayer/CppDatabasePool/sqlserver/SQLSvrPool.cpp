/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#include "SQLSvrPool.h"

using namespace std;

SQLSvrPool::SQLSvrPool(	string host, string database, string username, string password, unsigned int keepalivetimeout, string keepalivequery){
	this->username=username;
	this->password=password;
	this->database=database;
	this->host=host;
	this->keepalivequery=keepalivequery;
	this->keepalivetimeout=keepalivetimeout;

	if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlenvhandle)){
		ShowSQLError(cout, SQL_HANDLE_ENV, sqlenvhandle);
	}

	if(SQL_SUCCESS!=SQLSetEnvAttr(sqlenvhandle,SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0)){ 
		ShowSQLError(cout, SQL_HANDLE_ENV, sqlenvhandle);
	}
}

SQLHANDLE* SQLSvrPool::MyGetConcreteConnection(){
	SQLHANDLE* psqlconnectionhandle = (SQLHANDLE*)malloc(sizeof(SQLHANDLE));

	if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_DBC, sqlenvhandle, psqlconnectionhandle)){
		ShowSQLError(cout, SQL_HANDLE_DBC, *psqlconnectionhandle);
		return 0;
	}
	char sqlstring[1024];
	sprintf(sqlstring, "DRIVER={SQL Server};SERVER=%s, 1433;DATABASE=%s;UID=%s;PWD=%s;", host.c_str(), database.c_str(), username.c_str(), password.c_str());


	SQLCHAR retconstring[1024];
	switch(SQLDriverConnect (*psqlconnectionhandle, 
			NULL, 
			(SQLCHAR*)sqlstring,
			SQL_NTS, 
			retconstring, 
			1024, 
			NULL,
			SQL_DRIVER_NOPROMPT)){
		case SQL_INVALID_HANDLE:
		case SQL_ERROR:
			ShowSQLError(cout, SQL_HANDLE_DBC, *psqlconnectionhandle);
			return 0;
		case SQL_SUCCESS_WITH_INFO:
			ShowSQLError(cout, SQL_HANDLE_DBC, *psqlconnectionhandle);
		default:
			return psqlconnectionhandle;
	}
}

void SQLSvrPool::MyReleaseConcreteConnection(SQLHANDLE* psqlconnectionhandle){
	SQLDisconnect(*psqlconnectionhandle);
	SQLFreeHandle(SQL_HANDLE_DBC, *psqlconnectionhandle);
	free(psqlconnectionhandle);
}

int SQLSvrPool::MyPingConcreteConnection(SQLHANDLE* psqlconnectionhandle){
	SQLHANDLE sqlstatementhandle;
	if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, *psqlconnectionhandle, &sqlstatementhandle)){
		ShowSQLError(cout, SQL_HANDLE_STMT, sqlstatementhandle);
		return -1;
	}
	if(SQL_SUCCESS!=SQLExecDirect(sqlstatementhandle, (SQLCHAR*)keepalivequery.c_str(), SQL_NTS)){
		ShowSQLError(cout, SQL_HANDLE_STMT, sqlstatementhandle);
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle );
		return -1;
	}
	SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle );
	return 0;
}

int SQLSvrPool::MyGetPingTimeout(){
	return keepalivetimeout;
}

void SQLSvrPool::ShowSQLError(ostream& os, const unsigned int handletype, const SQLHANDLE& handle){
	SQLCHAR sqlstate[1024];
	SQLCHAR message[1024];
	if(SQL_SUCCESS == SQLGetDiagRec(handletype, handle, 1, sqlstate, NULL, message, 1024, NULL))
		os<<"Message: "<<message<<"\nSQLSTATE: "<<sqlstate<<endl;
	else
		os<<"Could not get error details";
}