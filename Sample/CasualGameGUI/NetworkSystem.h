#pragma once

class CasualGameManager;
class IPacketProtocol;
class BasePacket;

class NetworkSystem
{
public:
	NetworkSystem(void);
	virtual ~NetworkSystem(void);

	bool Intialize(CasualGameManager* pOwner, IPacketProtocol* pProtocol);
	bool Update();
	bool TCPSend(BasePacket* pPacket );
	bool UDPSend(unsigned char* pMessage, int bufSize );

	bool AddPeer(int serial, int externalIP, short externalPort, int localIP, short localPort);
	bool DeletePeer(int serial);
};

