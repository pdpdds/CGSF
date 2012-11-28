#include "TCPNetworkCallback.h"
#include "../Engine/Engine.h"

extern Engine *g_engine;

TCPNetworkCallback::TCPNetworkCallback(void)
{
}

TCPNetworkCallback::~TCPNetworkCallback(void)
{
}

bool TCPNetworkCallback::HandleNetworkMessage(BasePacket* pPacket)
{
	g_engine->GetCurrentState()->HandleNetworkMessage(pPacket);

	return true;
}

void TCPNetworkCallback::HandleConnect(int Serial)
{
	g_engine->SetLocalID(Serial);
}

void TCPNetworkCallback::HandleDisconnect(int Serial)
{
}