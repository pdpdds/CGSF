#pragma once

typedef struct tag_DBConnectionInfo
{
	TCHAR szDataSource[100]; 
	TCHAR szDBName[100];
	TCHAR szUser[100];
	TCHAR szPassword[100];
	TCHAR IP[100];
	USHORT Port;
}_DBConnectionInfo;