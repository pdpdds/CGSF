#pragma once
#include "INetworkFramework.h"

class ILogicEntry;

class SFIOCPFramework : public INetworkFramework
{
public:
	SFIOCPFramework(void);
	virtual ~SFIOCPFramework(void);

	virtual BOOL Initialize(eNetworkFrameworkType Type) override;
	virtual BOOL Start(ILogicEntry* pLogic) override;
	virtual BOOL End() override;
	virtual BOOL Finally() override;

	static char* GetModuleName(){return "IOCP";}
};
