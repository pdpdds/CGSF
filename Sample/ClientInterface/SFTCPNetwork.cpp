#include "stdafx.h"
#include "SFTCPNetwork.h"
#include "SFMacro.h"
#include "SFACEFramework.h"
#include "GoogleLog.h"
#include "SFClient.h"
#include "SFEngine.h"

SFTCPNetwork::SFTCPNetwork(void)
{
}


SFTCPNetwork::~SFTCPNetwork(void)
{
}

BOOL SFTCPNetwork::Initialize(INetworkCallback* pTCPCallBack)
{
	m_TCPClient = new SFSYSTEM_CLIENT;
	m_TCPClient->CreateSystem(); 

	SFClient* pTCPClient = m_TCPClient->GetNetworkPolicy();
	pTCPClient->SetNetworkCallback(pTCPCallBack);

	return TRUE;
}

BOOL SFTCPNetwork::Run()
{
	return m_TCPClient->Run(0);
}

BOOL SFTCPNetwork::Update()
{
	SFClient* pTCPClient = m_TCPClient->GetNetworkPolicy();
	return pTCPClient->Update();
}

BOOL SFTCPNetwork::Send(int Serial, USHORT PacketID, char* pMessage, int BufSize )
{
	SFClient* pTCPClient = m_TCPClient->GetNetworkPolicy();
	return pTCPClient->Send(Serial, PacketID, pMessage, BufSize);
}