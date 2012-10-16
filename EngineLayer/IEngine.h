#pragma once

class ILogicEntry;

class IEngine
{
public:
	IEngine(void){}
	virtual ~IEngine(void){}

	virtual BOOL CreateSystem(char* szModuleName, ILogicEntry* pLogic, bool Server = false) = 0;
	virtual BOOL Start(char* szIP, unsigned short Port) = 0;
	virtual BOOL ShutDown() = 0;
};