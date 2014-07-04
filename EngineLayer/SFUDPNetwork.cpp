#include "stdafx.h"
#include "SFUDPNetwork.h"
#include "OCFP2PClient/PuPeers.h"
#include "interface/P2PClient.h"
#include "OCFP2PClient/P2PManager.h"

#pragma warning (disable : 4244) 

HINSTANCE g_pP2PHandle = 0;

SFUDPNetwork::SFUDPNetwork()
: m_p2pModule(NULL)
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
	m_p2pModule = pfunc();

	m_p2pModule->Initialize(pCallback);

	return true;
}

BOOL SFUDPNetwork::Update()
{
	if (m_p2pModule)
		return m_p2pModule->Update();

	return FALSE;
}

bool SFUDPNetwork::Start()
{
	GetPrivateProfileStringA("UDPInfo", "IP", NULL, m_szIP, 20, "./Connection.ini");
	m_port = GetPrivateProfileIntA("UDPInfo", "PORT", 0, "./Connection.ini");

	if(m_p2pModule)
		m_p2pModule->RunP2P(m_szIP, m_port);

	return true;
}

bool SFUDPNetwork::Send(unsigned char* pData, unsigned int Length )
{
	m_p2pModule->BroadCast(pData, Length);

	return true;
}


bool SFUDPNetwork::AddPeer( int serial, int externalIP, short externalPort, int localIP, short localPort )
{
	PeerAddressInfo info;
	info.externalIP = externalIP;
	info.externalPort = externalPort;
	info.localIP = localIP;
	info.localPort = localPort;

	BYTE peerIndex = 0;
	m_p2pModule->AddPeer(&info, peerIndex);

	m_serialPeerMap.insert(std::make_pair(serial, peerIndex));

	return true;
}

bool SFUDPNetwork::DeletePeer( int serial )
{
	SerialPeerMap::iterator iter = m_serialPeerMap.find(serial);
	if(iter == m_serialPeerMap.end())
		return false;

	m_p2pModule->RemovePeer(iter->first);
	m_serialPeerMap.erase(iter->second);

	return true;
}