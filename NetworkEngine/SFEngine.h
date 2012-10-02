#pragma once
#include "SFFactory.h"
#include "IEngine.h"
#include "SFACEFramework.h"
#include "SFConfig.h"

class INetworkFramework;

using namespace CGSF;

template <typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>
class SFEngine : public IEngine
{
public:
	SFEngine(void);
	virtual ~SFEngine(void);

	virtual BOOL CreateSystem() override;
	virtual BOOL Run(ILogicEntry* pLogic) override;
	virtual BOOL Stop() override;

	INetworkFramework* m_pNetworkFramework;

	NetworkPolicy* GetNetworkPolicy(){return m_pNetworkPolicy;}

	BOOL GetProcessing(){return m_Processing;}
	void SetProcessing(BOOL Processing){m_Processing = Processing;}

	static SFConfig* GetConfig(){return &m_Config;}

protected:
	NetworkPolicy* m_pNetworkPolicy;
	LoggerPolicy* m_pLoggerPolicy;
	
private:
	SFFactory<INetworkFramework> m_SvrFactory;
	BOOL m_Processing;

	static SFConfig m_Config;
};

template <typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>
SFEngine<typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>::SFEngine(void)
{
	m_SvrFactory.Register(NetworkModulePolicy::GetModuleName(), (SFCreatorBase<INetworkFramework>*)(new SFCreator<INetworkFramework, NetworkModulePolicy>));

	m_pLoggerPolicy = new LoggerPolicy();
	m_pNetworkPolicy = new NetworkPolicy();

	m_Processing = FALSE;
}

template <typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>
SFEngine<typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>::~SFEngine(void)
{
	delete m_pLoggerPolicy;
	delete m_pNetworkPolicy;
}

template <typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>
BOOL SFEngine<typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>::CreateSystem()
{
	m_pNetworkFramework = m_SvrFactory.Create(NetworkModulePolicy::GetModuleName());
	
	return TRUE;
}

template <typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>
BOOL SFEngine<typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>::Run(ILogicEntry* pLogic)
{
	m_pLoggerPolicy->Initialize();

	m_Processing = TRUE;

	return m_pNetworkPolicy->Run(m_pNetworkFramework, pLogic);
}

template <typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>
BOOL SFEngine<typename NetworkPolicy, typename LoggerPolicy, typename NetworkModulePolicy>::Stop()
{
	m_pLoggerPolicy->Finally();

	return TRUE;
}