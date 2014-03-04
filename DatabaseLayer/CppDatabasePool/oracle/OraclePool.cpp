/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#include "OraclePool.h"
#include <iostream>

OraclePool::OraclePool(	string url, string username, string password, unsigned int keepalivetimeout, string keepalivequery){
	this->username=username;
	this->password=password;
	this->url=url;
	this->keepalivequery=keepalivequery;
	this->keepalivetimeout=keepalivetimeout;
	try{
		environment = oracle::occi::Environment::createEnvironment(oracle::occi::Environment::DEFAULT);
	}catch(oracle::occi::SQLException &e){
		std::cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__
		<<std::cout<<"\nDescription :"<<e.what()<<std::endl;
	}
}

oracle::occi::Connection* OraclePool::MyGetConcreteConnection(){
	oracle::occi::Connection* con;
	try{
		con = environment->createConnection(username, password, url);
		return con;
	}catch(oracle::occi::SQLException &e){
		std::cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__
		<<std::cout<<"\nDescription :"<<e.what()<<std::endl;
		return 0;
	}

}

void OraclePool::MyReleaseConcreteConnection(oracle::occi::Connection* con){
	environment->terminateConnection(con);
}

int OraclePool::MyPingConcreteConnection(oracle::occi::Connection *con){
	try{
	oracle::occi::Statement* stmt = con->createStatement(keepalivequery);
	oracle::occi::ResultSet* rs = stmt->executeQuery(keepalivequery);
	stmt->closeResultSet(rs);
    con->terminateStatement(stmt);
	return 0;

	}catch(oracle::occi::SQLException &e){
		std::cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__
		<<std::cout<<"\nDescription :"<<e.what()<<std::endl;
		return -1;
	}
}

int OraclePool::MyGetPingTimeout(){
	return keepalivetimeout;
}

