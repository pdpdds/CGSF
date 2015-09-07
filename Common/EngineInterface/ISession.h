#ifndef ISESSION_H_
#define ISESSION_H_

#include "INetworkEngine.h"
#include "IEngine.h"
#include "ISessionService.h"

#define SESSION_STATE_NULL 0
#define SESSION_STATE_CONNECT 1
#define SESSION_STATE_AUTENTICATE 2

class ISession
{
  public:  
	  ISession(){ m_sessionState = SESSION_STATE_NULL; m_channelNum = -1; }
    virtual ~ISession() {};

	void SetOwner(INetworkEngine* pOwner){m_pOwner = pOwner;}
	IEngine* GetEngine(){ return m_pOwner->GetEngine(); }

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

	void SetLoginState(int state){ m_sessionState = state; }
	int GetLoginState(){ return m_sessionState; }

	int m_channelNum;

protected:

private:
	INetworkEngine* m_pOwner;
	ISessionService* m_pSessionService;
	int m_sessionState;	
};


#endif  // ISESSION_H_