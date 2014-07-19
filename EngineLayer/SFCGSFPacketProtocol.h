#pragma once
#include "SFBasePacketProtocol.h"

class SFCommand;

class SFCGSFPacketProtocol : public SFBasePacketProtocol
{
public:
	SFCGSFPacketProtocol(void);
	virtual ~SFCGSFPacketProtocol(void);

	bool Initialize(int ioBufferSize, unsigned short packetDataSize, int packetOption);

	bool Reset();
	BasePacket* GetPacket(int& ErrorCode);
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool SendRequest(BasePacket* pPacket);
	bool DisposePacket(BasePacket* pPacket);

protected:

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};

