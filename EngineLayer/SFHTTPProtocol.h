#pragma once
#include "SFBasePacketProtocol.h"

class SFPacketIOBuffer;

class SFHTTPProtocol : public SFBasePacketProtocol
{
public:
	SFHTTPProtocol();
	virtual ~SFHTTPProtocol();

	bool Initialize(int ioBufferSize, unsigned short packetSize, int packetOption);
	bool Reset();

	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);

	BasePacket* GetPacket(int& errorCode);

	bool Encode(BasePacket* pPacket, char** ppBuffer, int& bufferSize);
	static bool DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();

protected:

private:
	SFIOBuffer* m_pIOBuffer;
};

