#pragma once
#include "PeerInfo.h"

class SFP2PSys
{
public:
	typedef std::map<int, _PeerInfo> PeerMap;
public:
	SFP2PSys(void);
	virtual ~SFP2PSys(void);

	BOOL AddPeer(int Serial, _PeerInfo& Info);
	BOOL DeletePeer(int Serial);
	BOOL ResetPeer();

	PeerMap& GetPeerMap(){return m_PeerMap;}

protected:

private:
	PeerMap m_PeerMap;
};
