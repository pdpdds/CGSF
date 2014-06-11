#pragma once

#define UDP_PKT_PUBLIC_IP_REQ	1000
#define UDP_PUBLIC_IP_RES		1001

#define UDP_PKT_PINGPONG_REQ	1002
#define UDP_PKT_PINGPONG_RES	1003

#define MAX_SDL_P2P_PACKET_SIZE 1024

typedef struct tag_NetPacketHeader
{
	unsigned int dataCRC;
	unsigned short directive;

}NetPacketHeader;

typedef struct tag_P2PPublicIP : public tag_NetPacketHeader
{ 
	unsigned int externalIP; 
	unsigned short externalPort;

}P2PPublicIP;

typedef struct tag_P2PPingPong : public tag_NetPacketHeader
{
	unsigned int sendTime;

}P2PPingPong;