#include "stdafx.h"
#include "SQLite3Test.h"
#include "CppSQLite3.h"
#include <ctime>
#include <fstream>
#include <iostream> 
#include <sstream>  

const char* gszFile = "test.db";

#pragma comment(lib, "SQLite3.lib")

using namespace std;

SQLite3Test::SQLite3Test(void)
{
}


SQLite3Test::~SQLite3Test(void)
{
}

bool SQLite3Test::Run()
{
	int i, fld;
	time_t tmStart, tmEnd;
	CppSQLite3DB db;

	cout << "SQLite Version: " << db.SQLiteVersion() << endl;

	remove(gszFile);
	db.open(gszFile);

	cout << endl << "emp table exists=" << (db.tableExists("emp") ? "TRUE":"FALSE") << endl;
	cout << endl << "Creating emp table" << endl;
	db.execDML("create table emp(empno int, empname char(20));");
	cout << endl << "emp table exists=" << (db.tableExists("emp") ? "TRUE":"FALSE") << endl;
	////////////////////////////////////////////////////////////////////////////////
	// Execute some DML, and print number of rows affected by each one
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "DML tests" << endl;
	int nRows = db.execDML("insert into emp values (7, 'David Beckham');");
	cout << nRows << " rows inserted" << endl;

	nRows = db.execDML("update emp set empname = 'Christiano Ronaldo' where empno = 7;");
	cout << nRows << " rows updated" << endl;

	nRows = db.execDML("delete from emp where empno = 7;");
	cout << nRows << " rows deleted" << endl;

	////////////////////////////////////////////////////////////////////////////////
	// Transaction Demo
	// The transaction could just as easily have been rolled back
	////////////////////////////////////////////////////////////////////////////////
	int nRowsToCreate(50000);
	cout << endl << "Transaction test, creating " << nRowsToCreate;
	cout << " rows please wait..." << endl;
	tmStart = time(0);
	db.execDML("begin transaction;");

	for (i = 0; i < nRowsToCreate; i++)
	{
		char buf[128];
		sprintf_s(buf, "insert into emp values (%d, 'Empname%06d');", i, i);
		db.execDML(buf);
	}

	db.execDML("commit transaction;");
	tmEnd = time(0);

	////////////////////////////////////////////////////////////////////////////////
	// Demonstrate CppSQLiteDB::execScalar()
	////////////////////////////////////////////////////////////////////////////////
	cout << db.execScalar("select count(*) from emp;") << " rows in emp table in ";
	cout << tmEnd-tmStart << " seconds (that was fast!)" << endl;

	////////////////////////////////////////////////////////////////////////////////
	// Re-create emp table with auto-increment field
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Auto increment test" << endl;
	db.execDML("drop table emp;");
	db.execDML("create table emp(empno integer primary key, empname char(20));");
	cout << nRows << " rows deleted" << endl;

	for (i = 0; i < 5; i++)
	{
		char buf[128];
		sprintf_s(buf, "insert into emp (empname) values ('Empname%06d');", i+1);
		db.execDML(buf);
		cout << " primary key: " << (int)db.lastRowId() << endl;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Query data and also show results of inserts into auto-increment field
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Select statement test" << endl;
	CppSQLite3Query q = db.execQuery("select * from emp order by 1;");

	for (fld = 0; fld < q.numFields(); fld++)
	{
		cout << q.fieldName(fld) << "(" << q.fieldDeclType(fld) << ")|";
	}
	cout << endl;

	while (!q.eof())
	{
		cout << q.fieldValue(0) << "|";
		cout << q.fieldValue(1) << "|" << endl;
		q.nextRow();
	}


	////////////////////////////////////////////////////////////////////////////////
	// SQLite's printf() functionality. Handles embedded quotes and NULLs
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "SQLite sprintf test" << endl;
	CppSQLite3Buffer bufSQL;
	bufSQL.format("insert into emp (empname) values (%Q);", "He's bad");
	cout << (const char*)bufSQL << endl;
	db.execDML(bufSQL);

	bufSQL.format("insert into emp (empname) values (%Q);", NULL);
	cout << (const char*)bufSQL << endl;
	db.execDML(bufSQL);

	////////////////////////////////////////////////////////////////////////////////
	// Fetch table at once, and also show how to use CppSQLiteTable::setRow() method
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "getTable() test" << endl;
	CppSQLite3Table t = db.getTable("select * from emp order by 1;");

	for (fld = 0; fld < t.numFields(); fld++)
	{
		cout << t.fieldName(fld) << "|";
	}
	cout << endl;
	for (int row = 0; row < t.numRows(); row++)
	{
		t.setRow(row);
		for (int fld = 0; fld < t.numFields(); fld++)
		{
			if (!t.fieldIsNull(fld))
				cout << t.fieldValue(fld) << "|";
			else
				cout << "NULL" << "|";
		}
		cout << endl;
	}


	////////////////////////////////////////////////////////////////////////////////
	// Test CppSQLiteBinary by storing/retrieving some binary data, checking
	// it afterwards to make sure it is the same
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Binary data test" << endl;
	db.execDML("create table bindata(desc char(10), data blob);");

	unsigned char bin[256];
	CppSQLite3Binary blob;

	for (i = 0; i < sizeof bin; i++)
	{
		bin[i] = i;
	}

	blob.setBinary(bin, sizeof bin);

	bufSQL.format("insert into bindata values ('testing', %Q);", blob.getEncoded());
	db.execDML(bufSQL);
	cout << "Stored binary Length: " << sizeof bin << endl;

	q = db.execQuery("select data from bindata where desc = 'testing';");

	if (!q.eof())
	{
		blob.setEncoded((unsigned char*)q.fieldValue("data"));
		cout << "Retrieved binary Length: " << blob.getBinaryLength() << endl;
	}
	q.finalize();

	const unsigned char* pbin = blob.getBinary();
	for (i = 0; i < sizeof bin; i++)
	{
		if (pbin[i] != i)
		{
			cout << "Problem: i: ," << i << " bin[i]: " << pbin[i] << endl;
		}
	}


	////////////////////////////////////////////////////////////////////////////////
	// Pre-compiled Statements Demo
	////////////////////////////////////////////////////////////////////////////////
	cout << endl << "Transaction test, creating " << nRowsToCreate;
	cout << " rows please wait..." << endl;
	db.execDML("drop table emp;");
	db.execDML("create table emp(empno int, empname char(20));");
	tmStart = time(0);
	db.execDML("begin transaction;");

	CppSQLite3Statement stmt = db.compileStatement("insert into emp values (?, ?);");
	for (i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf_s(buf, "EmpName%06d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);
		stmt.execDML();
		stmt.reset();
	}

	db.execDML("commit transaction;");
	tmEnd = time(0);

	cout << db.execScalar("select count(*) from emp;") << " rows in emp table in ";
	cout << tmEnd-tmStart << " seconds (that was even faster!)" << endl;
	cout << endl << "End of tests" << endl;

	return true;
}
