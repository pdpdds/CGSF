#pragma once
#include <map>

class IP2PManager;
class IUDPNetworkCallback;

enum eP2PWorkMode
{
	P2P_CLIENT = 0,
	P2P_SERVER,
};

class SFUDPNetwork
{
	typedef std::map<int, unsigned char> SerialPeerMap;
public:
	SFUDPNetwork();
	virtual ~SFUDPNetwork(void);

	bool Initialize(IUDPNetworkCallback* pCallback, TCHAR* szP2PModuleName);
	bool Start();
	BOOL Update();

	bool Send( unsigned char* pData, unsigned int Length);
	bool AddPeer(int Serial, int ExternalIP, short ExternalPort, int LocalIP, short LocalPort);
	bool DeletePeer(int Serial);

protected:

private:

	IP2PManager* m_P2PModule;
	SerialPeerMap m_SerialPeerMap;

	char m_IP[20];
	unsigned short m_Port;
};
