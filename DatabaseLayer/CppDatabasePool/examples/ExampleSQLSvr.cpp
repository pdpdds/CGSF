/* Copyright 2009 Righteous Ninja AKA P.S. Ching*/
#include <iostream>
#include "..\sqlserver\SQLSvrPool.h"

using namespace std;

int main(){
	


		SQLSvrPool* sqlsvrpool = new SQLSvrPool(	"localhost",	//hostnam
													"MyDatabase",	//database 
													"sa",			//username
													"Admin-123",	//password
													300000,		//keepalive timeout (milliseconds)
													"SELECT top 1 name FROM dbo.sysobjects");	//keepalive statement

		/*Create a pool that will have 3 cached connections and will swell upto a 
		  total of 5 connections. Returns the number of cached connections or -1 on error
		  */
		if(sqlsvrpool->CreatePool(3, 5)<=0){
			cout<<"Error creating database pool\n";
			cout<<sqlsvrpool->GetLastSystemError()<<endl;	//If it's asystem error
			goto EXAMPLE_END;
		}

		/*Dispaly the pool information*/
		cout<<(*sqlsvrpool);

		SQLHANDLE* psqlconnectionhandle;
		SQLHANDLE  sqlstatementhandle;
	

		/*Test the validity of the pool at anytime - not really required*/
		if(!sqlsvrpool->IsPoolValid())
			goto EXAMPLE_END;

		/*Get a connection from the pool*/
		if((psqlconnectionhandle=sqlsvrpool->GetConnectionFromPool())==0){
			cout<<"You have reached the maximum amout allowed - 5 in this example\n";
			goto EXAMPLE_END;
		}

		/*Get a statement handle from the connection*/
		if(SQL_SUCCESS!=SQLAllocHandle(SQL_HANDLE_STMT, *psqlconnectionhandle, &sqlstatementhandle)){
			sqlsvrpool->ShowSQLError(cout, SQL_HANDLE_DBC, *psqlconnectionhandle);
			goto EXAMPLE_END;
		}

		/*Execute the query and display the results. Do not Free the database connection handle.
		  This will be released via the ReleaseConnectionToPool back to the pool.
		 */
		if(SQL_SUCCESS!=SQLExecDirect(sqlstatementhandle, (SQLCHAR*)"select * from testtable", SQL_NTS)){
			sqlsvrpool->ShowSQLError(cout, SQL_HANDLE_STMT, sqlstatementhandle);
			goto EXAMPLE_END;
		}
		else{
			char name[64];
			char address[64];
			int id;
			while(SQLFetch(sqlstatementhandle)==SQL_SUCCESS){
				SQLGetData(sqlstatementhandle, 1, SQL_C_ULONG, &id, 0, NULL);
				SQLGetData(sqlstatementhandle, 2, SQL_C_CHAR, name, 64, NULL);
				SQLGetData(sqlstatementhandle, 3, SQL_C_CHAR, address, 64, NULL);
				cout<<id<<" "<<name<<" "<<address<<endl;
			}
		}
		
		SQLFreeHandle(SQL_HANDLE_STMT, sqlstatementhandle );


		/*Dispaly the pool information*/
		cout<<(*sqlsvrpool);


		/*Release the connection back into the pool*/
		sqlsvrpool->ReleaseConnectionToPool(psqlconnectionhandle);

		/*Dispaly the pool information*/
		cout<<(*sqlsvrpool);
		
		char c;
		cout<<"Enter character to exit\n";
		cin>>c;

EXAMPLE_END:

		/*Destroy the database pool*/
		if(sqlsvrpool->DestroyPool()>0){
			cout<<"There are still some un-released connections in the pool\n";
		}
		
		delete sqlsvrpool;




 return 0;
}

