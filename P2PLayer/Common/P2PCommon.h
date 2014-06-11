#pragma once

#define P2P_PINGPONG_INTERVAL 3000

typedef struct tag_PeerAddressInfo
{
	unsigned int localIP;
	unsigned short localPort;

	unsigned int externalIP;
	unsigned short externalPort;

}PeerAddressInfo;

typedef struct tag_P2PPeerInfo
{
	PeerAddressInfo peerAdressInfo;
	bool isConnected;
	unsigned int lastPingPongTime;
	unsigned int recvPacketTime;
	unsigned int recvPingPongCnt;

}P2PPeerInfo;

