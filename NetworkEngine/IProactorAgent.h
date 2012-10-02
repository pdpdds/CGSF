#pragma once

class SFProactorService;
class ILogicEntry;

class IProactorAgent
{
public:
	IProactorAgent(void);
	virtual ~IProactorAgent(void);

	virtual BOOL Start(ILogicEntry* pLogic) = 0;
	virtual BOOL End() = 0;

	virtual BOOL Run() = 0;

	virtual BOOL ServiceInitialize(SFProactorService* pService){return 0;}
	virtual BOOL ServiceDisconnect(int Serial){return FALSE;}

protected:
	int m_WorkThreadGroupID;
	int m_LogicThreadGroupID;

private:
};
