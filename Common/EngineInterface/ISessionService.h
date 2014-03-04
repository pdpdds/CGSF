#ifndef ISESSIONSERVICE_H_
#define ISESSIONSERVICE_H_
#include "IPacketProtocol.h"

class BaseBacket;
class ISession;

class ISessionService
{
  public:  
	  ISessionService(IPacketProtocol* pProtocol){m_pPacketProtocol = pProtocol;}
	  virtual ~ISessionService() {delete m_pPacketProtocol;};

	  virtual bool OnReceive(char* pData, unsigned short Length) = 0;
	 // virtual bool SendRequest(ISession* pSession, BasePacket* pPacket) = 0;

	  void SetSerial(int Serial){m_Serial = Serial;}

	  

protected:
	IPacketProtocol* m_pPacketProtocol;
	int m_Serial;

private:
	
};


#endif  // ISESSIONSERVICE_H_