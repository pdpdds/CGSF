#pragma once

#pragma warning( push )
#pragma warning( disable:4100 )

class BasePacket;

class INetworkCallback
{
public:
	INetworkCallback(void){ m_bConnected = false; m_serial = -1; }
	virtual ~INetworkCallback(void){}

	virtual bool HandleNetworkMessage(BasePacket* pPacket) = 0;
	virtual bool HandleRPC(BasePacket* pPacket){ return false; }
	virtual void HandleConnect(int serial, _SessionDesc m_sessiondesc)
	{
		printf("Connected\n"); 
		m_serial = serial;
		m_bConnected = true;
	}

	virtual void HandleDisconnect(int serial, _SessionDesc m_sessiondesc)
	{
		m_bConnected = false;
		printf("Disconnected\n");
	}
	bool IsConnected(){ return m_bConnected; }
	int GetSerial(){ return m_serial; }
	void SetSerial(int serial){ m_serial = serial; }

protected:

private:
	int m_serial;
	bool m_bConnected;
};

#pragma warning( pop )