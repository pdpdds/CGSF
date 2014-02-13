#include "stdafx.h"
#include "INITest.h"
#include "SFIni.h"

INITest::INITest(void)
{
}


INITest::~INITest(void)
{
}

bool INITest::Initialize()
{
	return true;
}

bool INITest::Run()
{
	SFIni ini;
	WCHAR szDatabaseName[MAX_PATH] = {0,}; 
	int port = 0; 

	ini.SetPathName(_T("./DataSource.ini")); 
	ini.GetString(L"DataSourceInfo",L"Database",szDatabaseName, MAX_PATH); 
	port = ini.GetInt(L"DataSourceInfo",L"PORT",0); 

	wprintf(L"Database Name : %s, Port : %d\n", szDatabaseName, port);

	return true;
}

bool INITest::Finally()
{
	return true;
}
