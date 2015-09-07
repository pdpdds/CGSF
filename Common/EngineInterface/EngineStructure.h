#pragma once
#include <string>

class ISession;

typedef struct tag_SessionDesc
{
	int sessionType; // 0 listener, 1 connector
	int identifier;
	std::string szIP;
	unsigned short port;
	ISession* pSession;

	tag_SessionDesc()
	{
		sessionType = -1;
		identifier = -1;		
		port = 0;
		pSession = 0;		
	}

}_SessionDesc;