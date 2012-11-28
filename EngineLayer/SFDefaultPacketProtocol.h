#pragma once

class SFCommand;

class SFDefaultPacketProtocol
{
public:
	SFDefaultPacketProtocol(void);
	virtual ~SFDefaultPacketProtocol(void);

	BOOL Reset();
	BasePacket* GetPacket(int& ErrorCode);
	BOOL AddTransferredData(char* pBuffer, DWORD dwTransferred);
	

protected:
	BOOL Initialize();

private:
	SFPacketIOBuffer* m_pPacketIOBuffer;
};



