// P2PClient.cpp : Defines the exported functions for the DLL application.
//
#include "../Interface/P2PClient.h"
#include "PuPeers.h"
#include "P2PManager.h"

static P2PManager g_P2PManager;

P2PCLIENT_API IP2PManager* GetP2PManager()
{
	return new P2PManager();
}