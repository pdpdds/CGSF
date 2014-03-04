/* Copyright 2009 Righteous Ninja AKA P.S. Ching*/
#include <iostream>
#include "..\oracle\OraclePool.h"

using namespace std;

int main(){
	
	try{

		OraclePool* oraclepool = new OraclePool(	"MYDATBASE",	//database (TNS)
													"gldbuser",		//username
													"gldbuser",		//password
													300000,		//keepalive timeout (milliseconds)
													"select 0 from dual");	//keepalive statement

		/*Create a pool that will have 3 cached connections and will swell upto a 
		  total of 5 connections. Returns the number of cached connections or -1 on error
		  */
		if(oraclepool->CreatePool(3, 5)<=0){
			cout<<"Error creating database pool\n";
			cout<<oraclepool->GetLastSystemError()<<endl;	//If it's asystem error
			goto EXAMPLE_END;
		}

		/*Dispaly the pool information*/
		cout<<(*oraclepool);

		oracle::occi::Connection *con;
		oracle::occi::Statement* stmt;
		oracle::occi::ResultSet* res;

		/*Test the validity of the pool at anytime - not really required*/
		if(!oraclepool->IsPoolValid())
			goto EXAMPLE_END;

		/*Get a connection from the pool*/
		if((con=oraclepool->GetConnectionFromPool())==0){
			cout<<"You have reached the maximum amout allowed - 5 in this example\n";
			goto EXAMPLE_END;
		}
		/*Select the schema and do a query. DO NOT delete the 'con' after the query.
		  This will be released via the ReleaseConnectionToPool back to the pool
		 */
		stmt = con->createStatement("select * from example");
		res = stmt->executeQuery();
		while (res->next())
			std::cout<<res->getInt(1)<<"  "<<res->getString(2)<<std::endl;
		
		stmt->closeResultSet(res);
		con->terminateStatement(stmt);

		/*Dispaly the pool information*/
		cout<<(*oraclepool);

		/*Release the connection back into the pool*/
		oraclepool->ReleaseConnectionToPool(con);

		/*Dispaly the pool information*/
		cout<<(*oraclepool);

		char c;
		cout<<"Enter character to exit\n";
		cin>>c;

EXAMPLE_END:

		/*Destroy the database pool*/
		if(oraclepool->DestroyPool()>0){
			cout<<"There are still some un-release connections in the pool\n";
		}
		
		delete oraclepool;

	}catch(oracle::occi::SQLException &e){
		std::cout<<e.what();
	}

 return 0;
}

