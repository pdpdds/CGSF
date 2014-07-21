#include "MGServerReceiver.h"
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/IEngine.h>
#include "BasePacket.h"

MGServerReceiver::MGServerReceiver(INetworkEngine* pOwner)
	: m_pOwner(pOwner)
{
}


MGServerReceiver::~MGServerReceiver(void)
{
}

void MGServerReceiver::notifyRegisterSocket(ASSOCKDESCEX& sockdesc, SOCKADDR_IN& ip)
{
	printf(" Connected %d\n", sockdesc.assockUid);

	ISession::OnConnect(sockdesc.assockUid);

	Synchronized es(&m_SessionLock);
	m_SessionMap.insert(std::make_pair(sockdesc.assockUid, sockdesc));
}

void MGServerReceiver::notifyReleaseSocket(ASSOCKDESCEX& sockdesc)
{
	printf("Disconnected %d\n", sockdesc.assockUid);

	ISession::OnDisconnect(sockdesc.assockUid);

	Synchronized es(&m_SessionLock);
           
	m_SessionMap.erase(sockdesc.assockUid);

	sockdesc.psender->releaseSocketUniqueId(sockdesc.assockUid);
}

void MGServerReceiver::notifyMessage(ASSOCKDESCEX& sockdesc, size_t length, char* data)
{
    if(false == ISession::OnReceive(data, length))
	{
		Synchronized es(&m_SessionLock);
           
		m_SessionMap.erase(sockdesc.assockUid);

		sockdesc.psender->releaseSocketUniqueId(sockdesc.assockUid);
	}
}

void MGServerReceiver::notifyConnectingResult(INT32 requestID, ASSOCKDESCEX& sockdesc, DWORD error)
{

}

bool MGServerReceiver::SendRequest(BasePacket* pPacket)
{
	IPacketProtocol* pProtocol = GetPacketProtocol();
	if (NULL == pProtocol)
		return false;

	char* pBuffer = NULL;
	int bufferSize;
	if (false == pProtocol->Encode(pPacket, &pBuffer, bufferSize))
		return false;

	Synchronized es(&m_SessionLock);

	SessionMap::iterator iter = m_SessionMap.find(pPacket->GetSerial());

	if (iter == m_SessionMap.end())
	{
		return false;
	}

	iter->second.psender->postingSend(iter->second, bufferSize, pBuffer);

	return true;
}

////////////////////////////////////////////////////////
//제대로 동작하는지 확인해 볼 필요가 있다...
////////////////////////////////////////////////////////
bool MGServerReceiver::Disconnect(int serial)
{
	Synchronized es(&m_SessionLock);

	SessionMap::iterator iter = m_SessionMap.find(serial);

	if(iter == m_SessionMap.end())
	{
		return FALSE;
	}

	iter->second.psender->releaseSocketUniqueId(serial);
           
	m_SessionMap.erase(serial);

	return true;
}

