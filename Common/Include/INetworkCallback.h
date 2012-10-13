#pragma once

class SFPacket;

class INetworkCallback
{
public:
	INetworkCallback(void){}
	virtual ~INetworkCallback(void){}

	virtual bool HandleNetworkMessage(int PacketID, BYTE* pBuffer, USHORT Length) = 0;
	virtual void HandleConnect(){printf("Connected\n");}
	virtual void HandleDisconnect(){printf("Disconnected\n");}
};