#pragma once
#include "ILogicEntry.h"

class LogicEntry : public ILogicEntry
{
public:
	LogicEntry(void);
	virtual ~LogicEntry(void);

	virtual BOOL Initialize() {return FALSE;}
	virtual BOOL ProcessPacket(BasePacket* pPacket);

	void SetLogic(ILogicEntry* pLogic)
	{
		m_pLogicEntry = pLogic;
	}

protected:

private:
	ILogicEntry* m_pLogicEntry;
};
