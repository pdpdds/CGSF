#pragma once

class INetworkFramework;

class SFServer
{
public:
	SFServer(void);
	virtual ~SFServer(void);

	BOOL Run(INetworkFramework* pFramework, ILogicEntry* pLogic);
	BOOL Stop();

	BOOL Send(int Serial, SFPacket* pPacket);
	BOOL Send( int Serial, USHORT PacketID, char* pBuffer, int BufferSize );

protected:

private:
	INetworkFramework* m_pFramework;
};
