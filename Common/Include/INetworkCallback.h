#pragma once

class BasePacket;

class INetworkCallback
{
public:
	INetworkCallback(void){}
	virtual ~INetworkCallback(void){}

	virtual bool HandleNetworkMessage(BasePacket* pPacket) = 0;
	virtual void HandleConnect(int Serial){printf("Connected\n");}
	virtual void HandleDisconnect(int Serial){printf("Disconnected\n");}
};