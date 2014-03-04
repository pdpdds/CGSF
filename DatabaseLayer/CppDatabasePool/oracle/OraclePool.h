/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#ifndef ORACLEPOOL_H
#define ORACLEPOOL_H
#include <string>

#include "..\CommonDatabasePool.cpp"
#include <occi.h>


class OraclePool : public CommonDatabasePool<oracle::occi::Connection>{
public:

	oracle::occi::Connection* MyGetConcreteConnection();
	void MyReleaseConcreteConnection(oracle::occi::Connection*);
	int MyPingConcreteConnection(oracle::occi::Connection*);
	int MyGetPingTimeout();
	OraclePool(string, string, string, unsigned int timeout=300000, string query="select 0 from dual");
private:
	oracle::occi::Environment* environment;
	string username;
	string password;
	string url;
	string keepalivequery;
	unsigned int keepalivetimeout;
};

#endif
