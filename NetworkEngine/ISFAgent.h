#pragma once

class SFPacket;

class ISFAgent
{
public:
	ISFAgent(void){}
	virtual ~ISFAgent(void){}

	virtual BOOL Start(ILogicEntry* pLogic) = 0;
	virtual BOOL End() = 0;
	virtual BOOL Run() = 0;

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) = 0;
	virtual BOOL Send(int Serial, SFPacket* pPacket) = 0;

};

