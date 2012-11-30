#pragma once

typedef struct tag_DBConnectionInfo
{
	CHAR szServiceName[100]; 
	CHAR szDBName[100];
	CHAR szID[100];
	CHAR szPassword[100];
	CHAR IP[100];
	USHORT Port;
}_DBConnectionInfo;