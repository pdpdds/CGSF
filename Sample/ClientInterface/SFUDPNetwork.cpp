#include "stdafx.h"
#include "SFUDPNetwork.h"
#include "PuPeers.h"
#include "P2PClient.h"
#include "P2PManager.h"
#include <windows.h>

HINSTANCE g_pP2PHandle = 0;

SFUDPNetwork::SFUDPNetwork()
: m_P2PModule(NULL)
{
	g_pP2PHandle = 0;
}

SFUDPNetwork::~SFUDPNetwork(void)
{
	if(g_pP2PHandle)
		::FreeLibrary(g_pP2PHandle);
}

bool SFUDPNetwork::Initialize(IUDPNetworkCallback* pCallback)
{
	g_pP2PHandle = ::LoadLibrary(L"P2PClient.dll");

	if(g_pP2PHandle == NULL)
		return false;

	GETP2PMANAGER_FUNC *pfunc;
	pfunc = (GETP2PMANAGER_FUNC *)::GetProcAddress( g_pP2PHandle, "GetP2PManager" );
	m_P2PModule = pfunc();

	m_P2PModule->Initialize(pCallback);

	return true;
}

BOOL SFUDPNetwork::Update()
{
	return m_P2PModule->Update();
}

bool SFUDPNetwork::Start()
{
	GetPrivateProfileStringA("UDPInfo", "IP", NULL, m_IP, 20, "./Connection.ini");
	m_Port = GetPrivateProfileIntA("UDPInfo", "PORT", 0, "./Connection.ini");

	m_P2PModule->RunP2P(m_IP, m_Port);

	return true;
}

bool SFUDPNetwork::Send(unsigned char* pData, unsigned int Length )
{
	m_P2PModule->BroadCast(pData, Length);

	return true;
}


bool SFUDPNetwork::AddPeer( int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort )
{
	PeerAddressInfo Info;
	Info.ExternalIP = ExternalIP;
	Info.ExternalPort = ExternalPort;
	Info.LocalIP = LocalIP;
	Info.LocalPort = LocalPort;

	BYTE PeerIndex = 0;
	m_P2PModule->AddPeer(&Info, PeerIndex);

	m_SerialPeerMap.insert(std::make_pair(Serial, PeerIndex));

	return true;
}

bool SFUDPNetwork::DeletePeer( int Serial )
{
	SerialPeerMap::iterator iter = m_SerialPeerMap.find(Serial);
	if(iter == m_SerialPeerMap.end())
		return false;

	m_P2PModule->RemovePeer(iter->first);
	m_SerialPeerMap.erase(iter->second);

	return true;
}