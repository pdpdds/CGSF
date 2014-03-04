#pragma once

class SFPlayer;
class BasePacket;

class SFSendDBRequest 
{
public:
	static BOOL RequestLogin(SFPlayer* pPlayer);

	static BOOL SendDBRequest(int RequestMsg, DWORD PlayerSerial, BasePacket* pPacket);
	static BOOL SendDBRequest(SFMessage* pMessage);

	static void SendToLogic(BasePacket* pMessage);

private:
	SFSendDBRequest(void){}
	~SFSendDBRequest(void){}
};

