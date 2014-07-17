#include "StdAfx.h"
#include "SFLogicEntry.h"
#include "SFPlayer.h"
#include "SFMacro.h"
#include "SFMySQLAdaptorImpl.h"
#include "SFPlayerManager.h"
#include "interface/P2PServer.h"

SFLogicEntry* SFLogicEntry::m_pLogicEntry = NULL;
HINSTANCE g_pP2PServerHandle = 0;

SFLogicEntry::SFLogicEntry(void)
: m_bP2PService(false)
{
	m_pLogicEntry = this;
}

bool SFLogicEntry::Initialize()
{
	SFDatabaseProxy* pProxyLocal = NULL;

	if(_tcscmp(SFDatabase::GetInfo()->szDataSource, _T("hsql")) == 0)
	{
		pProxyLocal = new SFDatabaseProxyLocal<SFMySQLAdaptorImpl>();
	}
	else
	{
		pProxyLocal = new SFDatabaseProxyLocal<SFMySQLAdaptorImpl>();
	}

	m_pDatabaseProxy = new SFDatabaseProxyImpl(pProxyLocal);
	m_pDatabaseProxy->Initialize();

	m_pRoomManager = new SFRoomManager();
	m_pRoomManager->Initialize();

	m_pPlayerManager = new SFPlayerManager();

	m_pLobby = new SFLobby();

	m_LogicContents.Load();

	m_pPlayerPool = new SFObjectPool<SFPlayer>(1000);

	CreateDirectoryWathcer();


///////////////////////////////////////////////////////////////////////////////
//p2p enable
///////////////////////////////////////////////////////////////////////////////
	SFIni ini;
	int p2pEnable = 0;
	USHORT listenPort = 0;
	WCHAR szP2PModule[MAX_PATH];

	ini.SetPathName(_T("./CasualGame.ini"));

	p2pEnable = ini.GetInt(L"P2P", L"ENABLE", 0);

	if (p2pEnable != 0)
	{
		ini.GetString(L"P2P", L"MODULE", szP2PModule, MAX_PATH);
		listenPort = ini.GetInt(L"P2P", L"PORT", 0);

		SetP2PService(true);

		g_pP2PServerHandle = ::LoadLibrary(szP2PModule);

		if (g_pP2PServerHandle == NULL)
		{
			LOG(ERROR) << "P2P Module" << szP2PModule << " Handle Create Fail!!";
			return false;
		}

		ACTIVATEP2P_FUNC *pfuncActivate;
		pfuncActivate = (ACTIVATEP2P_FUNC *)::GetProcAddress(g_pP2PServerHandle, "ActivateP2P");

		if (pfuncActivate == NULL)
		{
			LOG(ERROR) << "P2P Module " << szP2PModule << " => Can't find ActivateP2P Method!!";
			return false;
		}

		int result = pfuncActivate(listenPort);

		if (result != 0)
		{
			LOG(ERROR) << "P2P Module " << szP2PModule << " Activate fail!!";
			return false;
		}

		LOG(INFO) << "P2P Module " << szP2PModule << " Initialize Complete";
	}

	return true;
}

BOOL SFLogicEntry::CreateDirectoryWathcer()
{
	m_DirectoryWatcherTask.activate();

	return TRUE;
}

SFLogicEntry::~SFLogicEntry(void)
{
	if (g_pP2PServerHandle)
	{
		DEACTIVATEP2P_FUNC *pfuncDeactivate;
		pfuncDeactivate = (DEACTIVATEP2P_FUNC *)::GetProcAddress(g_pP2PServerHandle, "DeactivateP2P");
		int Result = pfuncDeactivate();

		::FreeLibrary(g_pP2PServerHandle);
	}

	if(m_pPlayerPool)
		delete m_pPlayerPool;

	if(m_pDatabaseProxy)
		delete m_pDatabaseProxy;

	if(m_pRoomManager)
		delete m_pRoomManager;

	if(m_pPlayerManager)
		delete m_pPlayerManager;

	if(m_pLobby)
		delete m_pLobby;
}

BOOL SFLogicEntry::AddGameMode(int Mode, SFGameMode* pMode)
{
	m_GameModeMap.insert(std::make_pair(Mode, pMode));
	return TRUE;
}

bool SFLogicEntry::ProcessPacket(BasePacket* pBase)
{
	switch (pBase->GetPacketType())
	{
	case SFPACKET_CONNECT:
		{	
			OnConnectPlayer(pBase->GetSerial());
		}
		break;

	case SFPACKET_DATA:
		{	
			OnPlayerData((BasePacket*)pBase);
		}
		break;

	case SFPACKET_TIMER:
		{	
			OnTimer((BasePacket*)pBase);
		}
		break;

	case SFPACKET_SHOUTER:
		{	
			OnShouter((SFPacket*)pBase);
		}
		break;


	case SFPACKET_DISCONNECT:
		{	
			OnDisconnectPlayer(pBase->GetSerial());
		}
		break;

	case SFPACKET_DB:
		{	
			OnDBResult((SFMessage*)pBase);
		}
		break;

	default:
		return false;
	}

	return true;
}

BOOL SFLogicEntry::OnConnectPlayer(int serial)
{
	SFPlayer* pPlayer = m_pPlayerPool->Alloc();

	if(pPlayer == NULL)
		return FALSE;

	pPlayer->Reset();
	pPlayer->SetSerial(serial);

	pPlayer->ChangeState(PLAYER_STATE_INIT);

	m_PlayerMap.insert(PlayerMap::value_type(serial, pPlayer));

	SendAuthPacket(serial);

	return TRUE;
}

BOOL SFLogicEntry::OnDisconnectPlayer(int serial)
{
	PlayerMap::iterator iter = m_PlayerMap.find(serial);
	
	if(iter == m_PlayerMap.end())
	{
		SFASSERT(0);
		return FALSE;
	}

	SFPlayer* pPlayer = iter->second;

	m_pPlayerManager->DelPlayer(pPlayer);

///////////////////////////////////////////////////////////
//20120619 이런식으로 짜면 실수할 여지가 크다.... 나중에 수정할 수 있도록 한다.
///////////////////////////////////////////////////////////
	if (pPlayer->GetPlayerState() == PLAYER_STATE_LOADING ||
		pPlayer->GetPlayerState() == PLAYER_STATE_ROOM ||
		pPlayer->GetPlayerState() == PLAYER_STATE_PLAY ||
		pPlayer->GetPlayerState() == PLAYER_STATE_PLAYEND ||
		pPlayer->GetPlayerState() == PLAYER_STATE_PLAYREADY)
	{
		SFRoomManager* pManager = GetRoomManager();
		SFRoom* pRoom = pManager->GetRoom(pPlayer->GetRoomIndex());
		pRoom->GetRoomFSM()->OnLeaveRoom(pPlayer);
	}

	pPlayer->ChangeState(PLAYER_STATE_NONE);

	m_pPlayerPool->Release(pPlayer);

	m_PlayerMap.erase(iter);

	return TRUE;
}

BOOL SFLogicEntry::OnPlayerData( BasePacket* pPacket )
{
	PlayerMap::iterator iter = m_PlayerMap.find(pPacket->GetSerial());

	if(iter == m_PlayerMap.end())
	{
		SFASSERT(0);
		return FALSE;
	}

	SFPlayer* pPlayer = iter->second;

	return pPlayer->ProcessPacket(pPacket);
}

BOOL SFLogicEntry::OnDBResult(SFMessage* pMessage)
{
	PlayerMap::iterator iter = m_PlayerMap.find(pMessage->GetSerial());

	if(iter == m_PlayerMap.end())
	{
		SFASSERT(0);
		return FALSE;
	}

	SFPlayer* pPlayer = iter->second;

	return pPlayer->ProcessDBResult(pMessage);
}

BOOL SFLogicEntry::OnTimer(BasePacket* pPacket)
{
	SFRoomManager* pManager = GetRoomManager();
	return pManager->Update();
}

BOOL SFLogicEntry::OnShouter(BasePacket* pPacket)
{
	return TRUE;
}

BOOL SFLogicEntry::SendRequest(BasePacket* pPacket)
{
	return SFEngine::GetInstance()->SendRequest(pPacket);
}
