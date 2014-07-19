#ifndef ISESSION_H_
#define ISESSION_H_

#include "INetworkEngine.h"
#include "IEngine.h"
#include "ISessionService.h"

class ISession
{
  public:  
	ISession(){}
    virtual ~ISession() {};

	void SetOwner(INetworkEngine* pOwner){m_pOwner = pOwner;}

	virtual void SendInternal(char* pBuffer, int BufferSize, int ownerSerial = -1) = 0;

	void OnConnect(int serial, int listenerId = 0, bool bServerToServerConnect = false)
	{
		m_pSessionService = m_pOwner->GetEngine()->CreateSessionService(bServerToServerConnect);
		m_pSessionService->SetSerial(serial);
		m_pOwner->GetEngine()->OnConnect(serial, listenerId);
	}

	void OnDisconnect(int serial, int listenerId = 0)
	{
		m_pOwner->GetEngine()->OnDisconnect(serial, listenerId);
		delete m_pSessionService;
	}

	bool OnReceive(char* pData, unsigned short length, int listenerId = 0)
	{
		return m_pSessionService->OnReceive(pData, length, listenerId);
	}

	/*
	bool SendRequest(BasePacket* pPacket)
	{
		return m_pSessionService->SendRequest(this, pPacket);
	}*/

protected:

private:
	INetworkEngine* m_pOwner;
	ISessionService* m_pSessionService;
};


#endif  // ISESSION_H_