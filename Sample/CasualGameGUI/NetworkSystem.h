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
	bool UDPSend(unsigned char* pMessage, int BufSize );

	bool AddPeer(int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort);
	bool DeletePeer(int Serial);
};

