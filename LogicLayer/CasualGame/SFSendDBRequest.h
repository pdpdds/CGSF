#pragma once

class SFPlayer;
class BasePacket;

class SFSendDBRequest 
{
public:
	static BOOL RequestLogin(SFPlayer* pPlayer);
	static BOOL SendRequest(int RequestMsg, DWORD PlayerSerial, BasePacket* pPacket);

	static SFMessage* GetInitMessage(int RequestMsg, DWORD PlayerSerial);
	static BOOL Send(SFMessage* pMessage);
	static void SendToLogic(BasePacket* pMessage);

private:
	SFSendDBRequest(void){}
	~SFSendDBRequest(void){}
};

