#include "P2PManager.h"
#include "SDL.h"
#include "SDL_net.h"
#include "../../BaseLayer/TinyThread/tinythread.h"
#include "../../BaseLayer/SFFastCRC.h"
#include <assert.h>
#include "../Interface/P2PData.h"
#include "liblfds611.h"
#include "../Common/SDLP2PCommon.h"

#ifdef WIN32
#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_Net.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblfds611.lib")
#endif

using namespace tthread;

IPaddress g_SDLServerIP;
static UDPsocket g_UDPSock = NULL;
static SDLNet_SocketSet g_SocketSet = NULL; //UDP 소켓 하나만 이용할 것임

void CleanUp();
void SDLNetThreadProc(void * aArg);
void HandleRecv();

thread* g_pSDLNetThread = 0;
bool g_bServerRunning = false;
static UDPpacket **g_Packets = NULL;

SFFastCRC g_FastCRC;

unsigned int GetP2PPacketCRC(char* pBuffer, int len)
{
	DWORD crc = 0;
	bool bResult = g_FastCRC.GetSimpleCRC((BYTE*)pBuffer, len, crc);

	if (bResult == true)
	{
		return crc;
	}

	return -1;
}

void CleanUp()
{
	if (g_UDPSock != NULL) {
		SDLNet_UDP_Close(g_UDPSock);
		g_UDPSock = NULL;
	}
	if (g_SocketSet != NULL) {
		SDLNet_FreeSocketSet(g_SocketSet);
		g_SocketSet = NULL;
	}

	SDLNet_Quit();
	SDL_Quit();
}

void SDLNetThreadProc(void * aArg)
{
	P2PManager* pManager = (P2PManager*)aArg;
	while (g_bServerRunning == true)
	{
		SDLNet_CheckSockets(g_SocketSet, 0);

		if (SDLNet_SocketReady(g_UDPSock)) {
			pManager->HandleRecv();
		}

		pManager->UpdateConnectionState();
	}
}

void P2PManager::HandleRecv()
{
	int n;

	n = SDLNet_UDP_RecvV(g_UDPSock, g_Packets);
	while (n-- > 0) {
		if (g_Packets[n]->channel >= 0) {
			if (g_Packets[n]->len > sizeof(NetPacketHeader))
			{
				NetPacketHeader packetHeader = *(NetPacketHeader*)(g_Packets[n]->data);
				DWORD dwCRC;

				BOOL bResult = g_FastCRC.GetZLibCRC((BYTE*)(g_Packets[n]->data + 4), g_Packets[n]->len - 4, dwCRC);

				if (bResult == TRUE && dwCRC == packetHeader.dataCRC)
				{
					switch (packetHeader.directive)
					{
					case UDP_PUBLIC_IP_RES:
						return ProcessPublicIP(g_Packets[n]);
					case UDP_PKT_PINGPONG_REQ:
						return ProcessPingPongReq(g_Packets[n]);
					case UDP_PKT_PINGPONG_RES:
						return ProcessPingPongRes(g_Packets[n]);
					}
				}
			}

		}
	}
}

void P2PManager::ProcessPublicIP(UDPpacket* pPacket)
{
	P2PPublicIP* pPublicIP = (P2PPublicIP*)pPacket->data;
	IPaddress* pMyIp = SDLNet_UDP_GetPeerAddress(g_UDPSock, -1);

	m_pUDPCallback->ReportMyIP(pMyIp->host, pMyIp->port, pPublicIP->externalIP, pPublicIP->externalPort);
}

void P2PManager::ProcessPingPongReq(UDPpacket* pPacket)
{
	NetPacketHeader* pHeader = (NetPacketHeader*)pPacket->data;
	pHeader->directive = UDP_PKT_PINGPONG_RES;
	EncodePacket(pPacket->data, pPacket, pPacket->len);

	SDLNet_UDP_Send(g_UDPSock, pPacket->channel, pPacket);
}

void P2PManager::ProcessPingPongRes(UDPpacket* pPacket)
{
	P2PPingPong* pPingPong = (P2PPingPong*)pPacket->data;
	P2PPeerInfo* pPeer = FindPeer(pPacket->channel);

	if (pPeer == NULL)
		return;

	pPeer->recvPacketTime = GetTickCount();
	pPeer->recvPingPongCnt++;
	pPeer->isConnected = true;
}

P2PPeerInfo* P2PManager::FindPeer(int channel)
{
	auto iter = m_mapPeer.find(channel);
	if (iter != m_mapPeer.end())
		return &iter->second;

	return NULL;
}


P2PManager::P2PManager(void)
{
	lfds611_queue_new(&m_pQueue, 1000);
	m_peerIndex = 0;
}

P2PManager::~P2PManager(void)
{
	lfds611_queue_delete(m_pQueue, NULL, NULL);
}

BOOL P2PManager::PushPacket(BYTE* pData, int Length)
{
	P2PData* pP2PData = new P2PData();
	pP2PData->Write(pData, Length);

	lfds611_queue_guaranteed_enqueue(m_pQueue, pP2PData);
	return TRUE;
}

BOOL P2PManager::Initialize(IUDPNetworkCallback* pCallback)
{
	m_pUDPCallback = pCallback;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		return FALSE;
	}

	if (SDLNet_Init() < 0)
		return FALSE;

	return TRUE;
}

BOOL P2PManager::Finally()
{
	CleanUp();
	return TRUE;
}

BOOL P2PManager::Update()
{
	P2PData* pP2PData = NULL;

	while (lfds611_queue_dequeue(m_pQueue, (void**)&pP2PData))
	{
		m_pUDPCallback->HandleUDPNetworkMessage(pP2PData->GetData(), pP2PData->GetDataSize());
		delete pP2PData;
		pP2PData = NULL;
	}

	return TRUE;
}

BOOL P2PManager::RunP2P(char* szIP, unsigned short usListenPort)
{
	SDLNet_ResolveHost(&g_SDLServerIP, 0, usListenPort);
	if (g_SDLServerIP.host == INADDR_NONE) {
		return FALSE;
	}

	g_UDPSock = SDLNet_UDP_Open(usListenPort);

	g_SocketSet = SDLNet_AllocSocketSet(1);
	if (g_SocketSet == NULL) {

		return FALSE;
	}

	SDLNet_UDP_AddSocket(g_SocketSet, g_UDPSock);

	g_bServerRunning = true;

	g_pSDLNetThread = new thread(SDLNetThreadProc, this);

	return TRUE;
}

BOOL P2PManager::EndP2P()
{
	g_bServerRunning = false;
	g_pSDLNetThread->join();
	delete g_pSDLNetThread;

	CleanUp();

	return TRUE;
}

BOOL P2PManager::AddPeer(PeerAddressInfo* pAddressInfo, BYTE& byIndex)
{
	P2PPeerInfo peerInfo;
	peerInfo.peerAdressInfo = *pAddressInfo;
	peerInfo.lastPingPongTime = GetTickCount();
	peerInfo.isConnected = false;
	peerInfo.recvPingPongCnt = 0;

	m_mapPeer.insert(std::make_pair(m_peerIndex, peerInfo));
	IPaddress newip;

	newip.host = pAddressInfo->externalIP;
	newip.port = pAddressInfo->externalPort;

	/* Put the address back in network form */
	newip.host = SDL_SwapBE32(newip.host);
	newip.port = SDL_SwapBE16(newip.port);

	/* Bind the address to the UDP socket */
	SDLNet_UDP_Bind(g_UDPSock, m_peerIndex, &newip);
	byIndex = m_peerIndex;

	m_peerIndex++;
	return TRUE;
}

BOOL P2PManager::RemovePeer(BYTE byIndex)
{
	m_mapPeer.erase(byIndex);
	SDLNet_UDP_Unbind(g_UDPSock, byIndex);
	return TRUE;
}

BOOL P2PManager::BroadCast(BYTE* pData, USHORT Size, BOOL ExceptMe)
{
	auto iter = m_mapPeer.begin();

	memcpy(g_Packets[0]->data, pData, Size);

	for (iter; iter != m_mapPeer.end(); iter++)
	{
		SDLNet_UDP_Send(g_UDPSock, iter->first, g_Packets[0]);
	}

	if (ExceptMe == false)
	{
		m_pUDPCallback->HandleUDPNetworkMessage(pData + sizeof(NetPacketHeader), Size - sizeof(NetPacketHeader));
	}

	return TRUE;
}

BOOL P2PManager::BroadCastWithRelayServer(BYTE* pData, USHORT Size)
{
	return TRUE;
}

BOOL P2PManager::Disconnect()
{
	CleanUp();
	return TRUE;
}

BOOL P2PManager::BroadcastEcho()
{
	return TRUE;
}


BOOL P2PManager::DataSend(const char* pszIP, USHORT uiPort, USHORT uiLen, const BYTE* pbyBuff)
{
	return TRUE;
}

bool P2PManager::UpdateConnectionState()
{
	auto iter = m_mapPeer.begin();

	DWORD dwCurTime = GetTickCount();

	for (iter; iter != m_mapPeer.end(); iter++)
	{		
		P2PPeerInfo &info = iter->second;

		if (info.isConnected == true)
		{
			if (dwCurTime - info.recvPacketTime > P2P_PINGPONG_INTERVAL)
				info.isConnected = false;
		}

		if (dwCurTime - info.lastPingPongTime > P2P_PINGPONG_INTERVAL)
		{
			info.lastPingPongTime = dwCurTime;
			SendUDPPingPongPacket(iter->first);
		}
	}

	return true;
}

bool P2PManager::SendUDPPingPongPacket(int channel)
{
	P2PPingPong pingPong;
	pingPong.directive = UDP_PKT_PINGPONG_REQ;
	pingPong.sendTime = GetTickCount();

	EncodePacket(&pingPong, g_Packets[0], sizeof(P2PPingPong));

	SDLNet_UDP_Send(g_UDPSock, channel, g_Packets[0]);
	return true;
}

bool P2PManager::EncodePacket(void* pData, UDPpacket* pPacket, int len)
{
	unsigned int crc = GetP2PPacketCRC((char*)pData + 4, len - 4);

	if (crc < -1)
		return false;

	NetPacketHeader* pHeader = (NetPacketHeader*)(pData);
	pHeader->dataCRC = crc;

	if (pPacket->data != (Uint8*)pHeader)
	{
		memcpy(pPacket->data, pData, len);
		pPacket->len = len;

	}	
	return true;
}


