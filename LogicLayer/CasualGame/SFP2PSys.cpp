#include "StdAfx.h"
#include "SFP2PSys.h"

SFP2PSys::SFP2PSys(void)
{
}

SFP2PSys::~SFP2PSys(void)
{
}

BOOL SFP2PSys::AddPeer(int serial, _PeerInfo& info)
{
	m_PeerMap.insert(std::make_pair(serial, info));

	//SFASSERT(0);

	return TRUE;
}

BOOL SFP2PSys::DeletePeer(int serial)
{
	m_PeerMap.erase(serial);

	return TRUE;
}

BOOL SFP2PSys::ResetPeer()
{
	m_PeerMap.clear();

	return TRUE;
}
