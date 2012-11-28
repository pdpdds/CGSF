#pragma once
#include <map>

class BasePacket;

class ILogicEntry
{
public:
	ILogicEntry(void){}
	virtual ~ILogicEntry(void){}

	virtual BOOL ProcessPacket(BasePacket* pPacket) = 0;
};
