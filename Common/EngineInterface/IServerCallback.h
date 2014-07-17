#pragma once

class BasePacket;

class IServerCallback
{
public:
	IServerCallback(void){m_isConnected = false; m_Serial = -1;}
	virtual ~IServerCallback(void){}

	virtual bool HandleServerMessage(int serverId, BasePacket* pPacket) = 0;

protected:

private:

};

