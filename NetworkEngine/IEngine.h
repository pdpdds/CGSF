#pragma once

class ILogicEntry;

class IEngine
{
public:
	IEngine(void){}
	virtual ~IEngine(void){}

	virtual BOOL CreateSystem() = 0;
	virtual BOOL Run(ILogicEntry* pLogic) = 0;
	virtual BOOL Stop() = 0;
};
