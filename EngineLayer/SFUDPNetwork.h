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
	bool AddPeer(int serial, int externalIP, short externalPort, int localIP, short localPort);
	bool DeletePeer(int serial);

protected:

private:

	IP2PManager* m_p2pModule;
	SerialPeerMap m_serialPeerMap;

	char m_szIP[20];
	unsigned short m_port;
};
