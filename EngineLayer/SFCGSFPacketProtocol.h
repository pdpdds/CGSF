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
	BOOL SendRequest(ISession* pSession, BasePacket* pPacket);

protected:
	BOOL Initialize();

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};

