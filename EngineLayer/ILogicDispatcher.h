#ifndef _ILOGICDISPATCHER_H_
#define  _ILOGICDISPATCHER_H_

class BasePacket;
class ILogicEntry;
class IRPCService;

class ILogicDispatcher
{  
public:  	 
	virtual void Dispatch(BasePacket* pPacket) = 0;

	virtual bool CreateLogicSystem(ILogicEntry* pLogicEntry) = 0;
	virtual bool ShutDownLogicSystem() = 0;

	virtual bool AddRPCService(IRPCService* pService) {return false;}
};

#endif // _ILOGICDISPATCHER_H_