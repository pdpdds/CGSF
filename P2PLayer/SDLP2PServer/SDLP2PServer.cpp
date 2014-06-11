#include "../Interface/P2PServer.h"
#include "SDL.h"
#include "SDL_net.h"
#include "../../BaseLayer/TinyThread/tinythread.h"
#include "../../BaseLayer/SFFastCRC.h"
#include "../Common/SDLP2PCommon.h"

#ifdef WIN32
#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2_Net.lib")
#pragma comment(lib, "zlib.lib")
#endif

using namespace tthread;

IPaddress g_SDLServerIP;
static UDPsocket g_UDPSock = NULL;
static SDLNet_SocketSet g_SocketSet = NULL; //UDP 소켓 하나만 이용할 것임

void CleanUp();
void SDLNetThreadProc(void * aArg);
void HandleClient();

thread* g_pSDLNetThread = 0;
bool g_bServerRunning = false;
static UDPpacket **g_Packets = NULL;

SFFastCRC g_FastCRC;

void ProcessPublicIP(UDPpacket* pPacket);

P2PSERVER_API int ActivateP2P(unsigned short usListenPort)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){		
		return -1;
	}

	if (SDLNet_Init() < 0)
		return -2;

	SDLNet_ResolveHost(&g_SDLServerIP, 0, usListenPort);
	if (g_SDLServerIP.host == INADDR_NONE) {
		return -3;
	}

	g_UDPSock = SDLNet_UDP_Open(usListenPort);
	
	g_SocketSet = SDLNet_AllocSocketSet(1);
	if (g_SocketSet == NULL) {

		return -4;
	}

	SDLNet_UDP_AddSocket(g_SocketSet, g_UDPSock);

	g_Packets = SDLNet_AllocPacketV(4, MAX_SDL_P2P_PACKET_SIZE);
	if (g_Packets == NULL)
	{
		return -5;
	}

	g_bServerRunning = true;

	g_pSDLNetThread = new thread(SDLNetThreadProc, 0);
	
	return 0;
}

P2PSERVER_API int DeactivateP2P()
{
	g_bServerRunning = false;
	g_pSDLNetThread->join();
	delete g_pSDLNetThread;

	CleanUp();

	return 0;
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
	while (g_bServerRunning == true)
	{
		SDLNet_CheckSockets(g_SocketSet, 0);

		if (SDLNet_SocketReady(g_UDPSock)) {
			HandleClient();
		}
	}
}

void HandleClient()
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
					switch(packetHeader.directive )
					{
					case UDP_PKT_PUBLIC_IP_REQ:
						return ProcessPublicIP(g_Packets[n]);
					}
				}
			}			
		}
	}
}

unsigned int GetP2PPacketCRC(char* pBuffer, int len)
{
	DWORD crc = 0;
	bool bResult = g_FastCRC.GetZLibCRC((BYTE*)pBuffer, len, crc);

	if (bResult == true)
	{
		return crc;
	}

	return -1;
}

void ProcessPublicIP(UDPpacket* pPacket)
{
	P2PPublicIP publicIP;
	publicIP.directive = UDP_PUBLIC_IP_RES;
	IPaddress externalAdress = pPacket->address;
	publicIP.externalIP = externalAdress.host;
	publicIP.externalPort = externalAdress.port;

	unsigned int crc = GetP2PPacketCRC(((char*)&publicIP) + 4, sizeof(P2PPublicIP) - 4);

	if (crc < -1)
		return;

	publicIP.dataCRC = crc;

	memcpy(g_Packets[0], &publicIP, sizeof(P2PPublicIP));

	SDLNet_UDP_Send(g_UDPSock, pPacket->channel, g_Packets[0]);
}