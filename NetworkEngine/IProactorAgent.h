#pragma once
#include "ISFAgent.h"

class SFProactorService;
class ILogicEntry;

class IProactorAgent : public ISFAgent
{
public:
	IProactorAgent(void);
	virtual ~IProactorAgent(void);

	virtual BOOL ServiceInitialize(SFProactorService* pService){return false;}
	virtual BOOL ServiceDisconnect(int Serial){return FALSE;}

protected:
	int m_WorkThreadGroupID;
	int m_LogicThreadGroupID;

private:
};
