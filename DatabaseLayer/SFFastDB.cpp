#include "stdafx.h"
#include "SFFastDB.h"
#include "fastdb.h"

extern dbDatabase db; // create database object

class tbllogin { 
public:
	char const* szUsername;
	char const* szPassword;
public:
	TYPE_DESCRIPTOR((FIELD(szUsername), FIELD(szPassword)));

};

REGISTER(tbllogin);

SFFastDB::SFFastDB(void)
{
}


SFFastDB::~SFFastDB(void)
{
	if(db.isOpen())
		db.close();
}

BOOL SFFastDB::Initialize(TCHAR* szDB)
{
	if(FALSE == db.open(szDB))
		return FALSE;

	return TRUE;
}

BOOL SFFastDB::Execute(char* szQuery)
{

	dbCursor<tbllogin> cursor;
	dbQuery q;

	char value[256];

	q = "szUsername=",value;
	gets(value);
	if (cursor.select(q) > 0) { 
		do { 
			printf("%s\n", cursor->szUsername);
		} while (cursor.next());
	}
	
	return TRUE;
}