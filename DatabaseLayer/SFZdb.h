#pragma once
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "zdb.h"
#ifdef __cplusplus
}
#endif

class SFZdb
{
public:
	SFZdb();	
	~SFZdb();

	BOOL Initialize(_DBConnectionInfo* pInfo);
	BOOL Execute(char* szQuery);

	URL_T url;
	ConnectionPool_T pool;
};