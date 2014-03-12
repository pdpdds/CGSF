#pragma once

class SFCommand;

class SFCGSFPacketProtocol
{
public:
	SFCGSFPacketProtocol(void);
	virtual ~SFCGSFPacketProtocol(void);

	BOOL Reset();
	BasePacket* GetPacket(int& ErrorCode);
	BOOL AddTransferredData(char* pBuffer, DWORD dwTransferred);
	bool SendRequest(BasePacket* pPacket);
	bool DisposePacket(BasePacket* pPacket);
	bool GetPacketData(BasePacket* pPacket, char* buffer, const int BufferSize, unsigned int& writtenSize);

protected:
	BOOL Initialize();

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};

