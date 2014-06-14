#pragma once
#include "ILogicDispatcher.h"

class SFCasualGameDispatcher : public ILogicDispatcher
{
public:
	SFCasualGameDispatcher(void);
	virtual ~SFCasualGameDispatcher(void);

	virtual void Dispatch(BasePacket* pPacket) override;
	virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) override;
	virtual bool ShutDownLogicSystem() override;

	virtual bool AddRPCService(IRPCService* pService) override;

	static void LogicThread(void* Args);
	static void RPCThread(void* Args);

	int GetLogicThreadCount(){ return m_nLogicThreadCnt; }
	void* GetLogicThreadFunc(){ return m_funcLogicThread; }

	int m_nLogicThreadCnt;
	void* m_funcLogicThread;

	void* GetRPCThreadFunc(){ return m_funcRPCThread; }
	void* m_funcRPCThread;

	IRPCService* m_pRPCService;

private:
	int m_LogicThreadGroupId;
	static bool m_bLogicEnd;
	
};

