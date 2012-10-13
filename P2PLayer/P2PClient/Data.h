#pragma once
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include <time.h>
class CData
{

public:
	static CData& GetInstance()
	{
		static CData Instance;
		return Instance;
	}

public:
	CData(void);
	~CData(void);

public:

	//SOCKADDR_IN		_stRemoteL;
	SOCKADDR_IN		_stRemoteE;


};
