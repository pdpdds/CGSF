#pragma once
#include <map>

class SFCommand;

class ILogicEntry
{
public:
	ILogicEntry(void){}
	virtual ~ILogicEntry(void){}

	virtual BOOL ProcessPacket(SFCommand* pCommand) = 0;
};
