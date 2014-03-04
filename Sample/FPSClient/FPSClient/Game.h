//-----------------------------------------------------------------------------
// The derived game state where all the game's processing happens.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#ifndef GAME_H
#define GAME_H

//-----------------------------------------------------------------------------
// State ID Define
//-----------------------------------------------------------------------------
#define STATE_GAME 1

//-----------------------------------------------------------------------------
// Enumerated Spawn Object Types
//-----------------------------------------------------------------------------
enum{ WEAPON_SPAWN_OBJECT };

//-----------------------------------------------------------------------------
// Custom Network Message Defines
//-----------------------------------------------------------------------------
/*#define MSGID_PLAYER_HEALTH          0x12005
#define MSGID_PLAYER_MOVE_UPDATE     0x12006
#define MSGID_PLAYER_LOOK_UPDATE     0x12007
#define MSGID_PLAYER_SCORE           0x12008
#define MSGID_PLAYER_WEAPON_CHANGE   0x12009
#define MSGID_PLAYER_WEAPON_CHANGING 0x12010
#define MSGID_SPAWN_POINT_REQUEST    0x12011
#define MSGID_SPAWN_POINT            0x12012
#define MSGID_SPAWN_PLAYER           0x12013
*/
//-----------------------------------------------------------------------------
// Game Class
//-----------------------------------------------------------------------------
class Game : public State
{
public:
	Game();

	virtual void Load();
	virtual void Close();

	virtual void RequestViewer( ViewerSetup *viewer );
	virtual void Render(float elapsed);

	BulletManager *GetBulletManager();

	//void HandleNetworkMessage( ReceivedMessage *msg );
	void HandleNetworkMessage( NetworkMessage* pMessage ) override;

/////////////////////////////////////////////////////////////////////////////////////
//20120606 박주항
//게임씬을 로드했다 하더라도 게임 시작전 같은 경우 입력을 허용하지 않을것이고
//게임이 끝나면 결과창을 보여주며 입력이 허용이 안되는 등 여러 상태가 존재할 것이므로
//네트워크 메세지와 업데이트 관련해서는 내부 상태에 따라 처리를 하도록 변경한다.
/////////////////////////////////////////////////////////////////////////////////////
	virtual void HandleNetworkMessage(BasePacket* pPacket) override;
	virtual void Update( float elapsed ) override;

private:
	Material *m_crosshair; // Material used to render the crosshair.

	char m_scoreBoardNames[MAX_PATH]; // Text for displaying the names of all the connected players.
	char m_scoreBoardFrags[MAX_PATH]; // Text for displaying each player's frag count.
	char m_scoreBoardDeaths[MAX_PATH]; // Text for displaying each player's death tally.
	Font *m_scoreBoardFont; // Font used to render the score board.

	BulletManager *m_bulletManager; // Bullet manager.
	PlayerManager *m_playerManager; // Player manager.

	Sound *m_music; // The in-game music sound.
	
};

//-----------------------------------------------------------------------------
// Externals
//-----------------------------------------------------------------------------
extern Game *g_game;

#endif