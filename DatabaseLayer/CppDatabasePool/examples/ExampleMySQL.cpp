/* Copyright 2009 Righteous Ninja AKA P.S. Ching*/
#include <iostream>
#include "..\mysql\MySQLPool.h"

using namespace std;

int main(){

	try{
		
		MySQLPool* mysqlpool = new MySQLPool(	"tcp://127.0.0.1:3306", //database url
												"root",					//username
												"root",					//password
												300000,		//keepalive timeout (milliseconds)
												"select 0 from dual");	//keepalive statement

		/*Create a pool that will have 3 cached connections and will swell upto a 
		  total of 5 connections. Returns the number of cached connections or -1 on error
		 */
		if(mysqlpool->CreatePool(3, 5)<=0){
			cout<<"Error creating database pool\n";
			cout<<mysqlpool->GetLastSystemError()<<endl; 	//If it's asystem error	
			goto EXAMPLE_END;
		}

		/*Dispaly the pool information*/
		cout<<(*mysqlpool);

		sql::Connection *con;
		sql::Statement* stmt;
		sql::ResultSet* res;

		/*Test the validity of the pool at anytime - not really required*/
		if(!mysqlpool->IsPoolValid())
			goto EXAMPLE_END;

		/*Get a connection from the pool*/
		if((con=mysqlpool->GetConnectionFromPool())==0){
			cout<<"You have reached the maximum amout allowed - 5 in this example\n";
			goto EXAMPLE_END;
		}
		/*Select the schema and do a query. DO NOT delete the 'con' after the query.
		  This will be released via the ReleaseConnectionToPool back to the pool
		  */
		con->setSchema("test");
		stmt = con->createStatement();
		res = stmt->executeQuery("select * from example");
		while (res->next())
			std::cout<<res->getInt("id")<<"  "<<res->getString("data")<<std::endl;

		
		/*Dispaly the pool information*/
		cout<<(*mysqlpool);

		/*Release the connection back into the pool*/
		mysqlpool->ReleaseConnectionToPool(con);

		/*Dispaly the pool information*/
		cout<<(*mysqlpool);

		char c;
		cout<<"Enter character to exit\n";
		cin>>c;

EXAMPLE_END:

		/*Destroy the database pool*/
		if(mysqlpool->DestroyPool()>0){
			cout<<"There are still some un-release connections in the pool\n";
		}

		delete mysqlpool;

	}catch(sql::SQLException &e){
		std::cout<<e.what();
	}


 return 0;
}

