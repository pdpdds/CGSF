#ifndef _IPACKETPROTOCOL_H
#define _IPACKETPROTOCOL_H

class ISession;

class IPacketProtocol
{
public:
	IPacketProtocol(){}
	virtual ~IPacketProtocol() {}

	virtual BOOL OnReceive(int Serial, char* pBuffer, DWORD dwTransferred) = 0;
	virtual BOOL SendRequest(ISession* pSession, BasePacket* pPacket) = 0;
	virtual IPacketProtocol* Clone() = 0;
};

#endif