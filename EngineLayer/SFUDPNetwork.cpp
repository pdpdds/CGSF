#include "stdafx.h"
#include "SFUDPNetwork.h"
#include "OCFP2PClient/PuPeers.h"
#include "interface/P2PClient.h"
#include "OCFP2PClient/P2PManager.h"

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

bool SFUDPNetwork::Initialize(IUDPNetworkCallback* pCallback, TCHAR* szP2PModuleName)
{
	g_pP2PHandle = ::LoadLibrary(szP2PModuleName);

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
	if(m_P2PModule)	
		return m_P2PModule->Update();

	return FALSE;
}

bool SFUDPNetwork::Start()
{
	GetPrivateProfileStringA("UDPInfo", "IP", NULL, m_IP, 20, "./Connection.ini");
	m_Port = GetPrivateProfileIntA("UDPInfo", "PORT", 0, "./Connection.ini");

	if(m_P2PModule)
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
	Info.externalIP = ExternalIP;
	Info.externalPort = ExternalPort;
	Info.localIP = LocalIP;
	Info.localPort = LocalPort;

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