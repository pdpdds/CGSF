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
	bool SendRequest(ISession* pSession, BasePacket* pPacket);
	bool DisposePacket(BasePacket* pPacket);

protected:
	BOOL Initialize();

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};

