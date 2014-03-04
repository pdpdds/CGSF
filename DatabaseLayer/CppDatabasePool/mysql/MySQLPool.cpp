/*Righteous Ninja AKA P.S. Ching codediaries.blogspot.com*/
#include "MySQLPool.h"

MySQLPool::MySQLPool(	string url, string username, string password, unsigned int keepalivetimeout, string keepalivequery){
	this->username=username;
	this->password=password;
	this->url=url;
	this->keepalivequery=keepalivequery;
	this->keepalivetimeout=keepalivetimeout;
	try{
		driver = get_driver_instance();
	}catch(sql::SQLException &e){
		std::cout<<"SQL error "<<e.getErrorCode()<<" at "<<__LINE__ << "\nDescription :"<< e.what() <<std::endl;
	}
}

sql::Connection* MySQLPool::MyGetConcreteConnection(){
	sql::Connection* con;
	try{
		con = driver->connect(url, username, password);
		return con;
	}catch(sql::SQLException &e){
		std::cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__ <<"\nDescription :"<<e.what()<<std::endl;
		return 0;
	}

}

void MySQLPool::MyReleaseConcreteConnection(sql::Connection* con){
	delete con;
}

int MySQLPool::MyPingConcreteConnection(sql::Connection *con){
	try{
	sql::Statement* stmt = con->createStatement();
	stmt->executeQuery(keepalivequery);
	delete stmt;
	return 0;
	}catch(sql::SQLException &e){
		std::cout<<"SQL error "<<e.getErrorCode()<<" at "<<__FUNCTION__ << "\nDescription :"<<e.what()<<std::endl;
		return -1;
	}
}

int MySQLPool::MyGetPingTimeout(){
	return keepalivetimeout;
}
