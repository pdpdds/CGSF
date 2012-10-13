#include "AllOcfClientSys.h"
#include "AllOcfClient.h"
#include "PuPeers.h"
#include "P2PManager.h"
#include "IUDPNetworkCallback.h"
#include <crtdbg.h>
#include <assert.h>
#include "P2PData.h"

#pragma comment(lib, "liblfds.lib")

P2PManager::P2PManager(void)
{	
	queue_new(&m_pQueue, 1000);
}

P2PManager::~P2PManager(void)
{
	queue_delete(m_pQueue, NULL, NULL);
}

BOOL P2PManager::RunP2P(char* szIP, unsigned short Port)
{
	GetPuCfgInstance().Init();
	GetPuLogInstance().Init();

	SetIdleTime(1000);
	SetSessionTimeout(8);//5

	GetPuCfgInstance().SetSerial( 0 );

	//
	if (THREADSTATUS_ZOMBIE == GetThreadStatus())
	{

		SetReceivePktQueue(FALSE);
		SetBindInfo("0.0.0.0", 0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);

		SetRelayInfo( szIP, Port );

		if (TRUE == CPuPeers::Start())
		{

			

			GetSelfLocalInfo(m_sinLocal);

			SendCheckExternalIp();
			SendCheckExternalIp();
		}
	}

	return TRUE;
}
	
BOOL P2PManager::EndP2P()		
{
	DoEnd();
	End();

	Uninit();

	return TRUE;
}

//피어를 더하는 즉시 피어들과 연결을 시도하며
//상대방이 릴라이어블 피어인지를 확인한다.(즉 내 패킷을 받을 수 있느냐)라는 의미
BOOL P2PManager::AddPeer(PeerAddressInfo* pAddressInfo, BYTE& byIndex)
{
	CPuPeers::AddPeer( pAddressInfo->LocalIP, 
		     pAddressInfo->LocalPort, 
			 pAddressInfo->ExternalIP, 
			 pAddressInfo->ExternalPort,
			 pAddressInfo->ExternalIP, 
			 pAddressInfo->ExternalPort,
			 byIndex );

	if (TRUE == GetConnectFlag())
	{
		TryConnectForPeers();
	}

	TryCheckRelayAblePeer();

	return TRUE;
}

BOOL P2PManager::BroadCast(BYTE* pData, USHORT Size, BOOL ExceptMe)
{
	if(ExceptMe == TRUE)	
		TryDataForPeers( Size, pData);
	else
	{
		TryDataForPeers( Size, pData);
		ProcessDataPeer(m_sinLocal, PU_PEERADDRTYPE_NONE, Size, pData);
	}

	return TRUE;
}

BOOL P2PManager::BroadCastWithRelayServer(BYTE* pData, USHORT Size)
{
	TryDataWithRelayForPeers(Size, pData);
	return TRUE;
}

BOOL P2PManager::DataSend( const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff)
{
	TryDataSend(pszIP, uiPort, uiLen, pbyBuff);
		
	return TRUE;
}

BOOL P2PManager::Disconnect()
{
	DoEnd();
	End();

	Uninit();

	return TRUE;
}

BOOL P2PManager::BroadcastEcho()
{
	if (TRUE == GetConnectFlag())
	{
		TryEchoForPeers();
	}

	return TRUE;
}

BOOL P2PManager::RemovePeer(BYTE byIndex)
{	
	DelPeer(byIndex);	

	return TRUE;
}

BOOL P2PManager::Initialize(IUDPNetworkCallback* pCallback)
{
	m_pUDPCallback = pCallback;

	return TRUE;
}

BOOL P2PManager::Finally()
{
	return TRUE;
}

BOOL P2PManager::Update()
{
	P2PData* pP2PData = NULL;

	while (queue_dequeue(m_pQueue,  (void**)&pP2PData))
	{
		m_pUDPCallback->HandleUDPNetworkMessage(pP2PData->GetData(), pP2PData->GetDataSize());
		delete pP2PData;
		pP2PData = NULL;
	}

	return TRUE;
}

void P2PManager::ProcessDataPeer(const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulLen, const BYTE* pbyBuff)
{
	printf("From [%d][%s, %d], DATA[%d]\n", enumAddrType, inet_ntoa(stRemote.sin_addr), ntohs(stRemote.sin_port), ulLen);	

//각 분기가 다른 쓰레드이지만 똑같이 Push해도 크게 문제는 없다.
	if(enumAddrType == PU_PEERADDRTYPE_NONE)
	{
		//m_pUDPCallback->HandleUDPNetworkMessage(pbyBuff, ulLen);
		PushPacket((BYTE*)pbyBuff, ulLen);
	}
	else
	{
		//m_pUDPCallback->HandleUDPNetworkMessage(pbyBuff + PU_PACKET_HEAD_LEN, ulLen - PU_PACKET_HEAD_LEN);
		PushPacket((BYTE*)(pbyBuff + PU_PACKET_HEAD_LEN), ulLen - PU_PACKET_HEAD_LEN);
	}
}

void P2PManager::OnResCheckExternalIp( const SOCKADDR_IN& stRemote, PU_PEERADDRTYPE enumAddrType, ULONG ulRemoteE, USHORT uiRemotePortE )
{
	//GetSelfLocalInfo

	//PlayerIPMsg Msg;
	m_pUDPCallback->ReportMyIP(ulRemoteE, uiRemotePortE, ulRemoteE, uiRemotePortE);
}

BOOL P2PManager::PushPacket(BYTE* pData, int Length)
{
	P2PData* pP2PData = new P2PData();
	pP2PData->Write(pData, Length);

	queue_guaranteed_enqueue(m_pQueue, pP2PData);
	return TRUE;
}