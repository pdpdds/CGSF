#include "stdafx.h"
#include "SFNetworkEngineCallback.h"
#include "SFUserSession.h"

SFNetworkEngineCallback::SFNetworkEngineCallback(void)
{
}

SFNetworkEngineCallback::~SFNetworkEngineCallback(void)
{
}

bool SFNetworkEngineCallback::OnConnect(int Serial)
{
	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Connect);
	pCommand->SetOwnerSerial(Serial);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

//20121015 오브젝트풀은 나중에
	SFUserSession* pUserSession = new SFUserSession(Serial);

	m_Lock.Lock();
	m_UserSessionMap.insert(std::make_pair(Serial, pUserSession));
	m_Lock.Unlock();

	return true;
}

bool SFNetworkEngineCallback::OnDisconnect(int Serial)
{
	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Disconnect);
	pCommand->SetOwnerSerial(Serial);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	m_Lock.Lock();
	UserSessionMap::iterator iter = m_UserSessionMap.find(Serial);

	if(iter != m_UserSessionMap.end())
	{
		SFUserSession* pSession = iter->second;
		m_UserSessionMap.erase(Serial);
		
		delete pSession;
	}

	m_Lock.Unlock();
	
	return true;
}

bool SFNetworkEngineCallback::OnData(int Serial, char* pData, unsigned short Length)
{
	UserSessionMap::iterator iter = m_UserSessionMap.find(Serial);

	if(iter == m_UserSessionMap.end())
	{
		SFASSERT(0);
		return false;
	}

	SFUserSession* pSession = iter->second;
	return pSession->ProcessData(pData, Length);
}

bool SFNetworkEngineCallback::OnTimer(const void *arg)
{
	SFPacket* pCommand = PacketPoolSingleton::instance()->Alloc();
	pCommand->SetPacketType(SFCommand_Timer);
	pCommand->SetOwnerSerial(-1);

	LogicGatewaySingleton::instance()->PushPacket(pCommand);

	printf("%d\n", arg);
	// Print out when timeouts occur.
	//  ACE_DEBUG ((LM_DEBUG, "(%t) %d timeout occurred for %s @ %d.\n",
	//        1,
	//        (char *) arg,
	//      (tv - this->start_time_).sec ()));

	return true;
}