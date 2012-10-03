#pragma once

class SFPlayer;

class SFSendDBRequest
{
public:
	static BOOL RequestLogin(SFPlayer* pPlayer, SFPacket* pPacket);
	static BOOL SendRequest(int RequestMsg, DWORD PlayerSerial, SFPacket* pPacket);

protected:
	static SFMessage* GetInitMessage(int RequestMsg, DWORD PlayerSerial);
	static BOOL Send(SFMessage* pMessage);

private:
	SFSendDBRequest(void){}
	~SFSendDBRequest(void){}
};

