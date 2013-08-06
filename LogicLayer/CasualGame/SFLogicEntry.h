#pragma once
#include "SFLogicContents.h"
#include "SFObjectPool.h"
#include "ILogicEntry.h"
#include "SFDataBaseProxy.h"
#include "SFDirectoryWatcherTask.h"
#include "SFPacketStore.pb.h"
#include "SFRoomManager.h"
#include "SFLobby.h"
#include "SFRoomFSM.h"
#include "SFDatabaseProxyLocal.h"
#include "SFDatabaseProxyImpl.h"
#include "SFEngine.h"
#include "SFMySQL.h"

///////////////////////////////////////////////////////////////////////////
//Logic Entry
//모든 컨텐츠 처리의 시작입니다.
//주의 : 이 클래스는 싱글턴으로 쓰입니다.
///////////////////////////////////////////////////////////////////////////
class SFPlayer;
class SFMessage;
class SFRoomManager;
class SFLobby;
class SFCommand;
class SFGameMode;

#include <map>

class SFLogicEntry : public ILogicEntry
{
public:
	typedef std::map<int, SFPlayer*> PlayerMap;
	typedef std::map<int, SFGameMode*> GameModeMap;

	SFLogicEntry(void);
	virtual ~SFLogicEntry(void);

	virtual BOOL Initialize() override;
	static SFLogicEntry* GetLogicEntry(){return m_pLogicEntry;}

	virtual BOOL ProcessPacket(BasePacket* pBase) override;
	
	BOOL CreateDirectoryWathcer();

	SFRoomManager* GetRoomManager(){return m_pRoomManager;}
	SFLobby* GetLobby(){return m_pLobby;}

	BOOL Send(SFPlayer* pPlayer, BasePacket* pPacket);
	BOOL SendRequest(BasePacket* pPacket);

	SFDatabaseProxy* GetDataBaseProxy(){return m_pDatabaseProxy;}

	BOOL AddGameMode(int Mode, SFGameMode* pMode);
	GameModeMap* GetGameModeMap(){return &m_GameModeMap;}

protected:
	BOOL OnConnectPlayer(int PlayerSerial);
	BOOL OnPlayerData(BasePacket* pPacket);
	BOOL OnTimer(BasePacket* pPacket);
	BOOL OnShouter(BasePacket* pPacket);

	BOOL OnDBResult(SFMessage* pMessage);
	BOOL OnDisconnectPlayer(int PlayerSerial);

private:
	static SFLogicEntry* m_pLogicEntry;

	SFObjectPool<SFPlayer>* m_pPlayerPool;
	PlayerMap m_PlayerMap;

	GameModeMap m_GameModeMap;
	
	SFRoomManager* m_pRoomManager;
	SFLobby* m_pLobby;

	SFDatabaseProxy* m_pDatabaseProxy;
	SFDirectoryWatcherTask m_DirectoryWatcherTask;

/////////////////////////////////////////////////////////////////////////////////////////////
//20120619 게임 처리 관련 필수 데이터를 담는 클래스
//서버 시작시 한번 로드되고 다시는 변경되지 않을 것이기에 쓰레드 세이프
//SFLogicEntry가 싱글턴으로 쓰이고 있지만 여러개 생성된다면 SFLogicContents는 따로 떼어내서 싱글톤으로 만들 필요 있음
/////////////////////////////////////////////////////////////////////////////////////////////
	SFLogicContents m_LogicContents;
};
