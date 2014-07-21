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

	void OnConnect(int serial, _SessionDesc& desc)
	{
		m_pSessionService = m_pOwner->GetEngine()->CreateSessionService(desc);
		m_pSessionService->SetSerial(serial);
		m_pOwner->GetEngine()->OnConnect(serial, desc);
	}

	void OnDisconnect(int serial, _SessionDesc& desc)
	{
		m_pOwner->GetEngine()->OnDisconnect(serial, desc);
		delete m_pSessionService;
	}

	bool OnReceive(char* pData, unsigned short length, _SessionDesc& desc)
	{
		return m_pSessionService->OnReceive(pData, length, desc);
	}

	virtual bool SendRequest(BasePacket* pPacket) = 0;

	IPacketProtocol* GetPacketProtocol(){ return m_pSessionService->GetPacketProtocol(); }

protected:

private:
	INetworkEngine* m_pOwner;
	ISessionService* m_pSessionService;
};


#endif  // ISESSION_H_