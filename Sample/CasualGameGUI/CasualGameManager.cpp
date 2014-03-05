#include "stdafx.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "CasualGameGUI.h"

CasualGameManager::CasualGameManager(void)
	: m_pNetworkSystem(0)
	, m_pGUISystem(0)
{
}


CasualGameManager::~CasualGameManager(void)
{
}

bool CasualGameManager::Initialize(LPDIRECT3DDEVICE9 pDevice, IPacketProtocol* pPacketProtocol)
{
	m_pNetworkSystem = new NetworkSystem();
	m_pGUISystem = new CasualGameGUI();

	m_pGUISystem->Create(pDevice);

	if (false == m_pNetworkSystem->Intialize(this, pPacketProtocol))
	{
		m_pGUISystem->QuitMessage("Network Intialize Fail!!");
		return false;
	}

	return true;
}

bool CasualGameManager::Finally()
{
	return true;
}

bool CasualGameManager::Render( float fElapsedTime )
{
	if(m_pGUISystem)
		m_pGUISystem->Render(fElapsedTime);

	return true;
}

bool CasualGameManager::Update()
{
	if(m_pNetworkSystem)
		m_pNetworkSystem->Update();

	return true;
}

bool CasualGameManager::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(m_pGUISystem)
		return m_pGUISystem->ProcessInput(uMsg, wParam, lParam);

	return false;
}