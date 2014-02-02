#ifndef _IPACKETPROTOCOL_H
#define _IPACKETPROTOCOL_H

class ISession;

class IPacketProtocol
{
public:
	IPacketProtocol(){}
	virtual ~IPacketProtocol() {}

	virtual bool OnReceive(int Serial, char* pBuffer, unsigned int dwTransferred) = 0;
	virtual bool SendRequest(ISession* pSession, BasePacket* pPacket) = 0;
	virtual IPacketProtocol* Clone() = 0;
};

#endif