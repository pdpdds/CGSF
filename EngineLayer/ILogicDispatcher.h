#ifndef _ILOGICDISPATCHER_H_
#define  _ILOGICDISPATCHER_H_

class BasePacket;

class ILogicDispatcher
{
  public:  
	virtual void Dispatch(BasePacket* pPacket) = 0;

	int GetLogicThreadCount(){ return m_nLogicThreadCnt; }
	void* GetBusinessThreadFunc(){ return m_funcBusnessThread;}

	int m_nLogicThreadCnt;
	void* m_funcBusnessThread;
};

#endif // _ILOGICDISPATCHER_H_