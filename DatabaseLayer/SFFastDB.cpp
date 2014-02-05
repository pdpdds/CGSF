#include "stdafx.h"
#include "SFFastDB.h"
#include "StringConversion.h"
#include "fastdb.h"

class tbllogin { 
public:
	char const* szUsername;
	char const* szPassword;
public:
	TYPE_DESCRIPTOR((FIELD(szUsername), FIELD(szPassword)));

};

REGISTER(tbllogin);

SFFastDB::SFFastDB(void)
	: m_pDatabase(NULL)
{
}


SFFastDB::~SFFastDB(void)
{
	if(m_pDatabase)
	{
		if(m_pDatabase->isOpen())
			m_pDatabase->close();

		delete m_pDatabase;
	}
	
}

BOOL SFFastDB::Initialize(TCHAR* szDB)
{
	m_pDatabase = new dbDatabase();
	std::string szDBSource = StringConversion::ToASCII(szDB);
	if(FALSE == m_pDatabase->open(szDBSource.c_str()))
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