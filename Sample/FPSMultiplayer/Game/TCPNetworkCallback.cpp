#include "TCPNetworkCallback.h"
#include "../Engine/Engine.h"

extern Engine *g_engine;

TCPNetworkCallback::TCPNetworkCallback(void)
{
}

TCPNetworkCallback::~TCPNetworkCallback(void)
{
}

bool TCPNetworkCallback::HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length)
{
	g_engine->GetCurrentState()->HandleNetworkMessage(PacketID, pBuffer, Length);

	return true;
}

void TCPNetworkCallback::HandleConnect(int Serial)
{
	g_engine->SetLocalID(Serial);
}

void TCPNetworkCallback::HandleDisconnect(int Serial)
{
}