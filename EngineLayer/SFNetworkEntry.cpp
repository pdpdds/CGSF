#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "SFTCPNetwork.h"
#include "SFUDPNetwork.h"
#include "SFIni.h"
#include "StringConversion.h"

#pragma comment(lib, "BaseLayer.lib")

SFNetworkEntry::SFNetworkEntry(void)
	: m_pTCPNetwork(NULL)
	, m_pUDPNetwork(NULL)
{
}


SFNetworkEntry::~SFNetworkEntry(void)
{
}

bool SFNetworkEntry::TCPSend(BasePacket* pPacket )
{
	if(m_pTCPNetwork)
		return (bool)m_pTCPNetwork->SendRequest(pPacket);

	return false;
}

bool SFNetworkEntry::UDPSend(unsigned char* pMessage, int BufSize )
{
	if(m_pUDPNetwork)
	return m_pUDPNetwork->Send((unsigned char*)pMessage, BufSize);

	return false;
}

BOOL SFNetworkEntry::Initialize(INetworkCallback* pTCPCallBack, IUDPNetworkCallback* pUDPCallback)
{
	if(pTCPCallBack == NULL)
		return FALSE;

	SFIni ini;
	WCHAR szEngine[MAX_PATH];

	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"Engine",L"NAME",szEngine, MAX_PATH);

	std::string str = StringConversion::ToASCII(szEngine);

	m_pTCPNetwork = new SFTCPNetwork();
	m_pTCPNetwork->Initialize((char*)str.c_str(), pTCPCallBack);

	if(pUDPCallback)
	{
		m_pUDPNetwork = new SFUDPNetwork();
		m_pUDPNetwork->Initialize(pUDPCallback);
	}
	
	return TRUE;
}

void SFNetworkEntry::SetLogicDispatcher(ILogicDispatcher* pDispatcher)
{
	m_pTCPNetwork->SetLogicDispatcher(pDispatcher);
}

void SFNetworkEntry::SetPacketProtocol(IPacketProtocol* pProtocol)
{
	m_pTCPNetwork->SetPacketProtocol(pProtocol);
}

BOOL SFNetworkEntry::Finally()
{
	if(m_pTCPNetwork)
		delete m_pTCPNetwork;

	return TRUE;
}

BOOL SFNetworkEntry::Run()
{
	SFIni ini;
	
	WCHAR szIP[20];
	USHORT Port;

	ini.SetPathName(_T("./Connection.ini"));
	ini.GetString(L"ServerInfo",L"IP",szIP, 20);
	Port = ini.GetInt(L"ServerInfo",L"PORT",0);

	std::string str = StringConversion::ToASCII(szIP);
	if(m_pTCPNetwork->Start((char*)str.c_str(), Port) == FALSE)
	{
		return FALSE;
	}

	if(m_pUDPNetwork)
	{
		m_pUDPNetwork->Start();
	}
	//char* pMessage = "cgsf";
	//m_pUDPNetwork->Send(( unsigned char*)pMessage, 5);
	
	return TRUE;
}

BOOL SFNetworkEntry::Update()
{
	m_pTCPNetwork->Update();

	if(m_pUDPNetwork)
		m_pUDPNetwork->Update();

	return TRUE;
}

BOOL SFNetworkEntry::IsConnected()
{
	return m_pTCPNetwork->IsConnected();
}

bool SFNetworkEntry::AddPeer(int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort)
{
	return m_pUDPNetwork->AddPeer(Serial, ExternalIP, ExternalPort, LocalIP, LocalPort);
}

bool SFNetworkEntry::DeletePeer(int Serial)
{
	return m_pUDPNetwork->DeletePeer(Serial);
}
