/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#ifndef MYSQLPOOL_H
#define MYSQLPOOL_H
#include <string>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "..\CommonDatabasePool.h"

using namespace std;


class MySQLPool : public CommonDatabasePool<sql::Connection>{
public:

	sql::Connection* MyGetConcreteConnection();
	void MyReleaseConcreteConnection(sql::Connection*);
	int MyPingConcreteConnection(sql::Connection*);
	int MyGetPingTimeout();
	MySQLPool(string, string, string, unsigned int timeout=300000, string query="select 0 from dual");
private:
	sql::Driver* driver;
	string username;
	string password;
	string url;
	string keepalivequery;
	unsigned int keepalivetimeout;
};

#endif
