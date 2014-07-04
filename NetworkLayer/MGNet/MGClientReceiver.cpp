#include "MGClientReceiver.h"
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/IEngine.h>
#include "MGEngine.h"


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

	ISession::OnDisconnect(sockdesc.assockUid);

	Synchronized es(&m_sessionLock);
           
	m_sessionMap.erase(sockdesc.assockUid);

	sockdesc.psender->releaseSocketUniqueId(sockdesc.assockUid);
}

void MGClientReceiver::notifyMessage(ASSOCKDESCEX& sockdesc, size_t length, char* data)
{
	if(false == ISession::OnReceive(data, length))
	{
		Synchronized es(&m_sessionLock);
           
		m_sessionMap.erase(sockdesc.assockUid);

		sockdesc.psender->releaseSocketUniqueId(sockdesc.assockUid);
	}
}

void MGClientReceiver::notifyConnectingResult(INT32 requestID, ASSOCKDESCEX& sockdesc, DWORD error)
{
	if(error == 0)
	{
		printf(" Connected %d\n", sockdesc.assockUid);

		ISession::OnConnect(sockdesc.assockUid);

		Synchronized es(&m_sessionLock);
		m_sessionMap.insert(std::make_pair(sockdesc.assockUid, sockdesc));
	}
	
}

void MGClientReceiver::SendInternal(char* pBuffer, int BufferSize, int ownerSerial)
{
	Synchronized es(&m_sessionLock);

	SessionMap::iterator iter = m_sessionMap.find(ownerSerial);

	if(iter == m_sessionMap.end())
	{
		return;
	}

	iter->second.psender->postingSend(iter->second, BufferSize, pBuffer);
}

////////////////////////////////////////////////////////
//제대로 동작하는지 확인해 볼 필요가 있다...
////////////////////////////////////////////////////////
bool MGClientReceiver::Disconnect(int serial)
{
	Synchronized es(&m_sessionLock);

	SessionMap::iterator iter = m_sessionMap.find(serial);

	if(iter == m_sessionMap.end())
	{
		return FALSE;
	}

	iter->second.psender->releaseSocketUniqueId(serial);
           
	m_sessionMap.erase(serial);

	return true;

}