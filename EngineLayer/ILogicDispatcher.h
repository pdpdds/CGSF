#ifndef _ILOGICDISPATCHER_H_
#define  _ILOGICDISPATCHER_H_

class BasePacket;

class ILogicDispatcher
{
  public:  
	virtual void Dispatch(BasePacket* pPacket) = 0;
};

#endif // _ILOGICDISPATCHER_H_