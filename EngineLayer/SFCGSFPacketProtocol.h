#pragma once

class SFCommand;

class SFCGSFPacketProtocol
{
public:
	SFCGSFPacketProtocol(void);
	virtual ~SFCGSFPacketProtocol(void);

	bool Initialize(int ioBufferSize, USHORT packetDataSize);

	bool Reset();
	BasePacket* GetPacket(int& ErrorCode);
	bool AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool SendRequest(BasePacket* pPacket);
	bool DisposePacket(BasePacket* pPacket);
	bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize);

protected:

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};

