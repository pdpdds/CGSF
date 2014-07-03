#ifndef _IPACKETPROTOCOL_H
#define _IPACKETPROTOCOL_H

class ISession;
class BasePacket;

class IPacketProtocol
{
public:
	IPacketProtocol(){}
	virtual ~IPacketProtocol() {}

	virtual bool OnReceive(int Serial, char* pBuffer, unsigned int dwTransferred, int acceptorId) = 0;
	virtual bool SendRequest(BasePacket* pPacket) = 0;
	//virtual BasePacket* CreatePacket() = 0;
	virtual bool DisposePacket(BasePacket* pPacket) = 0;

	virtual IPacketProtocol* Clone() = 0;
};

#endif