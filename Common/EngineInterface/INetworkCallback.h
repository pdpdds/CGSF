#pragma once

class BasePacket;

class INetworkCallback
{
public:
	INetworkCallback(void){m_isConnected = false; m_Serial = -1;}
	virtual ~INetworkCallback(void){}

	virtual bool HandleNetworkMessage(BasePacket* pPacket) = 0;
	virtual void HandleConnect(int Serial)
	{
//		printf("Connected\n"); 
		m_Serial = Serial;
		m_isConnected = true;
	}

	virtual void HandleDisconnect(int Serial)
	{
		m_isConnected = false;
	//	printf("Disconnected\n");
	}
	bool IsConnected(){return m_isConnected;}
	int GetSerial(){return m_Serial;}
	void SetSerial(int serial){m_Serial = serial;}

protected:

private:
	int m_Serial;
	bool m_isConnected;
};