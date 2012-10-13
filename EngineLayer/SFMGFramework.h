#pragma once
#include "INetworkFramework.h"

class ILogicEntry;

class SFMGFramework : public INetworkFramework
{
public:
	SFMGFramework();
	virtual ~SFMGFramework(void);

	virtual BOOL Initialize(eNetworkFrameworkType Type) override;
	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;
	virtual BOOL Finally() override;

	static char* GetModuleName(){return "MG";}

	virtual BOOL Send(int Serial, USHORT PacketID, char* pMessage, int BufSize ) override;
	virtual BOOL Send(int Serial, SFPacket* pPacket) override;

protected:

};

