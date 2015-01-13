#include "stdafx.h"
#include "SFZdb.h"
#include <XML/StringConversion.h>

SFZdb::SFZdb()
{
	
}

SFZdb::~SFZdb()
{
}

BOOL SFZdb::Initialize(_DBConnectionInfo* pInfo)
{
	std::string szDBUrl = "mysql://" + StringConversion::ToASCII(pInfo->IP) + ":" + StringConversion::ToASCII(pInfo->Port) + "/" +
		StringConversion::ToASCII(pInfo->szDBName).c_str() + "?" +
		"user=" +
		StringConversion::ToASCII(pInfo->szUser).c_str() + "&" +
		"password=" +
		StringConversion::ToASCII(pInfo->szPassword).c_str();
		
	url = URL_new(szDBUrl.c_str());
	pool = ConnectionPool_new(url);
	ConnectionPool_start(pool);

	return TRUE;
}

BOOL SFZdb::Execute(char* szQuery)
{
	return TRUE;
}

