#ifndef ISESSIONSERVICE_H_
#define ISESSIONSERVICE_H_
#include "IPacketProtocol.h"

class BasePacket;
class ISession;

class ISessionService
{
  public:  
	  ISessionService(IPacketProtocol* pProtocol){m_pPacketProtocol = pProtocol;}
	  virtual ~ISessionService() {delete m_pPacketProtocol;};

	  virtual bool OnReceive(char* pData, unsigned short Length, _SessionDesc& desc) = 0;
	 // virtual bool SendRequest(ISession* pSession, BasePacket* pPacket) = 0;

	  void SetSerial(int serial){m_serial = serial;}

	  IPacketProtocol* GetPacketProtocol(){ return m_pPacketProtocol; }

protected:
	IPacketProtocol* m_pPacketProtocol;
	int m_serial;

private:
	
};


#endif  // ISESSIONSERVICE_H_