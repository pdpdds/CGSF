#include "StdAfx.h"
#include "SFP2PSys.h"

SFP2PSys::SFP2PSys(void)
{
}

SFP2PSys::~SFP2PSys(void)
{
}

BOOL SFP2PSys::AddPeer(int Serial, _PeerInfo& Info)
{
	m_PeerMap.insert(std::make_pair(Serial, Info));

	//SFASSERT(0);

	return TRUE;
}

BOOL SFP2PSys::DeletePeer(int Serial)
{
	m_PeerMap.erase(Serial);

	return TRUE;
}

BOOL SFP2PSys::ResetPeer()
{
	m_PeerMap.clear();

	return TRUE;
}
