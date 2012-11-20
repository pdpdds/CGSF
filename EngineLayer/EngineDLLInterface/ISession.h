#ifndef ISESSION_H_
#define ISESSION_H_
#include "INetworkEngine.h"
#include "INetworkEngineCallback.h"
#include "IPacketService.h"

class ISession
{
  public:  
	ISession(){}
    virtual ~ISession() {};

	void SetOwner(INetworkEngine* pOwner){m_pOwner = pOwner;}

	//void OnConnect(int Serial);
	//void OnDisconnect(int Serial);
	//bool OnData(int Serial, char* pData, unsigned short Length);

	void OnConnect(int Serial)
	{
		m_pPacketService = m_pOwner->GetCallback()->CreatePacketService();
		m_pOwner->GetCallback()->OnConnect(Serial);
	}

	void OnDisconnect(int Serial)
	{
		m_pOwner->GetCallback()->OnDisconnect(Serial);
		delete m_pPacketService;
	}

	bool OnData(int Serial, char* pData, unsigned short Length)
	{
		return m_pPacketService->OnData(Serial, pData, Length);
	}

protected:

private:
	INetworkEngine* m_pOwner;
	IPacketService* m_pPacketService;
};


#endif  // ISESSION_H_