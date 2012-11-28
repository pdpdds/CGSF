#pragma once

class SFPlayer;

class SFSendDBRequest
{
public:
	static BOOL RequestLogin(SFPlayer* pPlayer);
	static BOOL SendRequest(int RequestMsg, DWORD PlayerSerial, BasePacket* pPacket);

protected:
	static SFMessage* GetInitMessage(int RequestMsg, DWORD PlayerSerial);
	static BOOL Send(SFMessage* pMessage);

private:
	SFSendDBRequest(void){}
	~SFSendDBRequest(void){}
};

