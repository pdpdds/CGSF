#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "SFTCPNetwork.h"
#include "SFUDPNetwork.h"
#include "SFIni.h"
#include "StringConversion.h"

SFNetworkEntry::SFNetworkEntry(void)
	: m_pTCPNetwork(NULL)
	, m_pUDPNetwork(NULL)
{
}


SFNetworkEntry::~SFNetworkEntry(void)
{
}

bool SFNetworkEntry::TCPSend(int Serial,  USHORT PacketID, char* pMessage, int BufSize )
{
	if(m_pTCPNetwork)
		return m_pTCPNetwork->Send(Serial, PacketID, pMessage, BufSize);

	return false;
}

bool SFNetworkEntry::UDPSend(unsigned char* pMessage, int BufSize )
{
	if(m_pUDPNetwork)
	return m_pUDPNetwork->Send((unsigned char*)pMessage, BufSize);

	return false;
}

BOOL SFNetworkEntry::Initialize(char* szModuleName, INetworkCallback* pTCPCallBack, IUDPNetworkCallback* pUDPCallback)
{
	if(pTCPCallBack == NULL)
		return FALSE;

	m_pTCPNetwork = new SFTCPNetwork();
	m_pTCPNetwork->Initialize(szModuleName, pTCPCallBack);

	if(pUDPCallback)
	{
		m_pUDPNetwork = new SFUDPNetwork();
		m_pUDPNetwork->Initialize(pUDPCallback);
	}
	
	return TRUE;

}

BOOL SFNetworkEntry::Finally()
{
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

bool SFNetworkEntry::AddPeer(int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort)
{
	return m_pUDPNetwork->AddPeer(Serial, ExternalIP, ExternalPort, LocalIP, LocalPort);
}

bool SFNetworkEntry::DeletePeer(int Serial)
{
	return m_pUDPNetwork->DeletePeer(Serial);
}
