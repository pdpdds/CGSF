#pragma once

class SFPlayer;

class SFSendDBRequest
{
public:
	static BOOL SendRequest(int RequestMsg, DWORD PlayerSerial, SFPacket* pPacket);
	static SFMessage* GetInitMessage(int RequestMsg, DWORD PlayerSerial);

	static BOOL RequestLogin(SFPlayer* pPlayer, SFPacket* pPacket);

protected:

private:
	SFSendDBRequest(void){}
	~SFSendDBRequest(void){}
};

