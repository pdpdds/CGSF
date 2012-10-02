#pragma once

class INetworkFramework;

class SFServer
{
public:
	SFServer(void);
	virtual ~SFServer(void);

	BOOL Run(INetworkFramework* pFramework, ILogicEntry* pLogic);
};
