#pragma once

class SFCommand;

class SFCGSFPacketProtocol
{
public:
	SFCGSFPacketProtocol(void);
	virtual ~SFCGSFPacketProtocol(void);

	bool Initialize(int ioBufferSize = PACKET_DEFAULT_IO_SIZE, USHORT packetDataSize = PACKET_DEFAULT_PACKET_SIZE);

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

