#include "stdafx.h"
#include "SFFastDB.h"
#include "StringConversion.h"
#include "fastdb.h"

/*
class tbllogin { 
public:
	char const* szUsername;
	char const* szPassword;
public:
	TYPE_DESCRIPTOR((FIELD(szUsername), FIELD(szPassword)));

};

REGISTER(tbllogin);
*/

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
	if(FALSE == m_pDatabase->open(szDB))
		return FALSE;

	printf("SFFastDB Init With %s\n", szDBSource.c_str());

	return TRUE;
}