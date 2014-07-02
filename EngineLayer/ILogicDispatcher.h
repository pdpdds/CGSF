#ifndef _ILOGICDISPATCHER_H_
#define  _ILOGICDISPATCHER_H_

#pragma warning( push )
#pragma warning( disable:4100 ) 

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

#pragma warning( pop )

#endif // _ILOGICDISPATCHER_H_