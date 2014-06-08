#ifndef _ILOGICDISPATCHER_H_
#define  _ILOGICDISPATCHER_H_

class BasePacket;

class ILogicDispatcher
{
  public:  
	virtual void Dispatch(BasePacket* pPacket) = 0;

	int GetLogicThreadCount(){ return m_nLogicThreadCnt; }
	void* GetBusinessThreadFunc(){ return m_funcBusnessThread;}
	void* GetRPCThreadFunc(){ return m_funcRPCThread; }

	int m_nLogicThreadCnt;
	void* m_funcBusnessThread;
	void* m_funcRPCThread;
};

#endif // _ILOGICDISPATCHER_H_