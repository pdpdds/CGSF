#pragma once
#include "INetworkFramework.h"

class ILogicEntry;

class SFACEFramework : public INetworkFramework
{
public:
	SFACEFramework();
	virtual ~SFACEFramework(void);

	virtual BOOL Initialize(eNetworkFrameworkType Type) override;
	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;
	virtual BOOL Finally() override;

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) override;
	virtual BOOL Send(int Serial, SFPacket* pPacket) override;

	static char* GetModuleName(){return "ACE";}

protected:

};