#pragma once
#include "SFDatabase.h"

class dbDatabase;

class SFFastDB //: public SFDatabase
{
public:
	SFFastDB(void);
	~SFFastDB(void);

	BOOL Initialize(TCHAR* szDB);
	BOOL Execute(char* szQuery);

protected:

private:
	dbDatabase* m_pDatabase; // create database object
};

