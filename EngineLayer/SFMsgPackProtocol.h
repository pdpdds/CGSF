#pragma once
#include <msgpack.hpp>

class SFMsgPackPacket;

class SFMsgPackProtocol
{
public:
	SFMsgPackProtocol();
	virtual ~SFMsgPackProtocol();

	bool Reset();
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	BasePacket* GetPacket(int& errorCode);
	bool SendRequest(BasePacket* pPacket);
	bool DisposePacket(BasePacket* pPacket);
	BasePacket* CreatePacket();
	bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize);

protected:

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;

	bool Initialize();
};

