#include "MGClientReceiver.h"
#include "INetworkEngine.h"
#include "MGEngine.h"
#include "INetworkEngineCallback.h"

MGClientReceiver::MGClientReceiver(INetworkEngine* pOwner)
	: m_pOwner(pOwner)
{
}


MGClientReceiver::~MGClientReceiver(void)
{
}

void MGClientReceiver::notifyRegisterSocket(ASSOCKDESCEX& sockdesc, SOCKADDR_IN& ip)
{
	/*printf(" Connected %d\n", sockdesc.assockUid);

	m_pOwner->GetCallback()->OnConnect(sockdesc.assockUid);

	Synchronized es(&m_SessionLock);
	m_SessionMap.insert(std::make_pair(sockdesc.assockUid, sockdesc));*/
}

void MGClientReceiver::notifyReleaseSocket(ASSOCKDESCEX& sockdesc)
{
	printf("Disconnected %d\n", sockdesc.assockUid);

	m_Session.OnDisconnect(sockdesc.assockUid);

	Synchronized es(&m_SessionLock);
           
	m_SessionMap.erase(sockdesc.assockUid);

	sockdesc.psender->releaseSocketUniqueId(sockdesc.assockUid);
}

void MGClientReceiver::notifyMessage(ASSOCKDESCEX& sockdesc, size_t length, char* data)
{
	if(false == m_Session.OnData(sockdesc.assockUid,data, length))
	{
		Synchronized es(&m_SessionLock);
           
		m_SessionMap.erase(sockdesc.assockUid);

		sockdesc.psender->releaseSocketUniqueId(sockdesc.assockUid);
	}
}

void MGClientReceiver::notifyConnectingResult(INT32 requestID, ASSOCKDESCEX& sockdesc, DWORD error)
{
	if(error == 0)
	{
		printf(" Connected %d\n", sockdesc.assockUid);

		m_Session.OnConnect(sockdesc.assockUid);

		Synchronized es(&m_SessionLock);
		m_SessionMap.insert(std::make_pair(sockdesc.assockUid, sockdesc));
	}
	
}

BOOL MGClientReceiver::Send(int Serial, char* pMessage, int BufSize )
{
	Synchronized es(&m_SessionLock);

	SessionMap::iterator iter = m_SessionMap.find(Serial);

	if(iter == m_SessionMap.end())
	{
		return FALSE;
	}

	iter->second.psender->postingSend(iter->second, BufSize, pMessage);

	return TRUE;
}

////////////////////////////////////////////////////////
//제대로 동작하는지 확인해 볼 필요가 있다...
////////////////////////////////////////////////////////
bool MGClientReceiver::Disconnect(int Serial)
{
	Synchronized es(&m_SessionLock);

	SessionMap::iterator iter = m_SessionMap.find(Serial);

	if(iter == m_SessionMap.end())
	{
		return FALSE;
	}

	iter->second.psender->releaseSocketUniqueId(Serial);
           
	m_SessionMap.erase(Serial);

	return true;

}