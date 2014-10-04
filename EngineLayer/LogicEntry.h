#pragma once
#include "ILogicEntry.h"

class LogicEntry
{
public:
	LogicEntry(void);
	virtual ~LogicEntry(void);

	virtual bool ProcessPacket(BasePacket* pPacket);
	virtual bool DestroyLogic();

	void SetLogic(ILogicEntry* pLogic)
	{
		m_pLogicEntry = pLogic;
	}

protected:

private:
	ILogicEntry* m_pLogicEntry;
};
