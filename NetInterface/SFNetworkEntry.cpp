#include "stdafx.h"
#include "SFNetworkEntry.h"
#include "SFTCPNetwork.h"
#include "SFUDPNetwork.h"

SFNetworkEntry::SFNetworkEntry(void)
	: m_pTCPNetwork(NULL)
	, m_pUDPNetwork(NULL)
{
}


SFNetworkEntry::~SFNetworkEntry(void)
{
}

bool SFNetworkEntry::TCPSend( USHORT PacketID, char* pMessage, int BufSize )
{
	if(m_pTCPNetwork)
		return m_pTCPNetwork->Send(PacketID, pMessage, BufSize);

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

	m_pTCPNetwork = new SFTCPNetwork();
	m_pTCPNetwork->Initialize(pTCPCallBack);

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
	if(m_pTCPNetwork->Run() == FALSE)
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
