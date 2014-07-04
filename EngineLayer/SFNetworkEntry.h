#pragma once

class SFTCPNetwork;
class SFUDPNetwork;
class INetworkCallback;
class IUDPNetworkCallback;
class BasePacket;
class ILogicDispatcher;
class IPacketProtocol;
class IRPCInterface;

class SFNetworkEntry
{
public:
	SFNetworkEntry(void);
	virtual ~SFNetworkEntry(void);

	static SFNetworkEntry* GetInstance();
	bool Initialize(INetworkCallback* pTCPCallBack, IPacketProtocol* pProtocol, ILogicDispatcher* pDispatcher = NULL, IUDPNetworkCallback* pUDPCallback = NULL);
	bool ShutDown();

	bool Run();
	bool Update();
	bool IsConnected();

	bool TCPSend(BasePacket* pPacket );
	bool UDPSend(unsigned char* pMessage, int BufSize );
	//bool TCPSend( USHORT PacketID, NetworkMessage* pMessage, int Size );

	SFTCPNetwork* GetTCPNetwork(){return m_pTCPNetwork;}
	SFUDPNetwork *GetUDPNetwork(){return m_pUDPNetwork;}

	bool AddPeer(int serial, int externalIP, short externalPort, int localIP, short localPort);
	bool DeletePeer(int serial);


protected:

private:
	static SFNetworkEntry* m_pNetworkEntry;
	SFTCPNetwork* m_pTCPNetwork; //TCP
	SFUDPNetwork *m_pUDPNetwork; // UDP
};