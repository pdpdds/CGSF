#pragma once

typedef struct tag_PeerInfo
{
	DWORD LocalIP;
	USHORT LocalPort;

	DWORD ExternalIP;
	USHORT ExternalPort;

	tag_PeerInfo()
	{
		LocalIP = 0;
		LocalPort = 0;

		ExternalIP = 0;
		ExternalPort = 0;

	}

}_PeerInfo;