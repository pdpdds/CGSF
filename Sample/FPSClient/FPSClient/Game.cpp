//-----------------------------------------------------------------------------
// Game.h implementation.
// Refer to the Game.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "FPSMain.h"
#include "GamePacketStructure.h"
#include <Windows.h>
#include <SFPacketStore/PacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "PeerInfo.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "BasePacket.h"

extern CasualGameManager* g_pCasualGameManager;

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
Game *g_game = NULL;

//-----------------------------------------------------------------------------
// Game class constructor.
//-----------------------------------------------------------------------------
Game::Game() : State( STATE_GAME )
{
	// Store a pointer to the game state in a global variable for easy access.
	g_game = this;

	// Invalidate the score board font.
	m_scoreBoardFont = NULL;

	// Invalidate both the bullet manager and the player manager.
	m_bulletManager = NULL;
	m_playerManager = NULL;

	// Invalidate the in-game music sound.
	m_music = NULL;
}

//-----------------------------------------------------------------------------
// Allows the game state to preform any pre-processing construction.
//-----------------------------------------------------------------------------
void Game::Load()
{
	// Hide the mouse cursor.
	ShowCursor( false );

	// Load the crosshair material.
	m_crosshair = g_engine->GetMaterialManager()->Add( "Crosshair.dds.txt", "./Assets/" );

	// Create the score board font.
	m_scoreBoardFont = new Font( "Arial", 14, FW_BOLD );
	m_scoreBoardNames[0] = 0;
	m_scoreBoardFrags[0] = 0;
	m_scoreBoardDeaths[0] = 0;

	// Create both the bullet manager and the player manager.
	m_bulletManager = new BulletManager;
	m_playerManager = new PlayerManager;

	// Load and play the in-game music.
	m_music = new Sound( "./Assets/Sounds/music_loop.wav" );
	m_music->Play( true );

	if(g_engine->GetGameStateBlock())	 
		g_engine->GetGameStateBlock()->Apply();

	// Load the scene from the host player's selection.
	g_engine->GetSceneManager()->LoadScene( "Abandoned City.txt", "./Assets/Scenes/" );

	/*SFProtobufPacket<SFPacketStore::LoadingComplete> PktLoadingComplete(CGSF::LoadingComplete);
	PktLoadingComplete.SetOwnerSerial(g_engine->GetLocalID());

	g_pCasualGameManager->GetNetwork()->TCPSend(&PktLoadingComplete);*/

	ShowCursor( true );
}

//-----------------------------------------------------------------------------
// Allows the game state to preform any post-processing destruction.
//-----------------------------------------------------------------------------
void Game::Close()
{
	// Show the mouse cursor.
	//ShowCursor( true );

	// Terminate the session.
	//g_engine->GetNetwork()->Terminate();

	// Destroy the scene.
	g_engine->GetSceneManager()->DestroyScene();

	// Destroy both the bullet manager and the player manager.
	SAFE_DELETE( m_bulletManager );
	SAFE_DELETE( m_playerManager );

	// Destroy the score board font.
	SAFE_DELETE( m_scoreBoardFont );

	// Destroy the crosshair material.
	g_engine->GetMaterialManager()->Remove( &m_crosshair );

	// Destroy the in-game music.
	SAFE_DELETE( m_music );
}

//-----------------------------------------------------------------------------
// Returns the view setup details for the given frame.
//-----------------------------------------------------------------------------
void Game::RequestViewer( ViewerSetup *viewer )
{
	viewer->viewer = m_playerManager->GetViewingPlayer();
	viewer->viewClearFlags = D3DCLEAR_ZBUFFER;
}

//-----------------------------------------------------------------------------
// Update the game state.
//-----------------------------------------------------------------------------
void Game::Update( float elapsed )
{
	// Allow the player to respawn by pressing the R key.
	/*if( g_engine->GetInput()->GetKeyPress( DIK_R ) == true )
	{
		m_playerManager->GetLocalPlayer()->SetEnabled( false );
		m_playerManager->SpawnLocalPlayer();
	}*/

	// Update the bullet manager before the player manager. This will prevent
	// bullets that are fired this frame from being processed this frame.

	

	m_bulletManager->Update( elapsed );

	// Update the player manager.
	m_playerManager->Update( elapsed );

	// Check if the user is holding down the tab key.
	if( g_engine->GetInput()->GetKeyPress( DIK_TAB, true ) == true )
	{
		// Build the score board text.
		sprintf( m_scoreBoardNames, "PLAYER\n" );
		sprintf( m_scoreBoardFrags, "FRAGS\n" );
		sprintf( m_scoreBoardDeaths, "DEATHS\n" );

		// Add each player's details to the score board.
		PlayerObject *player = m_playerManager->GetNextPlayer( true );
		while( player != NULL )
		{
			strcat( m_scoreBoardNames, player->GetName() );
			strcat( m_scoreBoardNames, "\n" );

			sprintf( m_scoreBoardFrags, "%s%d", m_scoreBoardFrags, player->GetFrags() );
			strcat( m_scoreBoardFrags, "\n" );

			sprintf( m_scoreBoardDeaths, "%s%d", m_scoreBoardDeaths, player->GetDeaths() );
			strcat( m_scoreBoardDeaths, "\n" );

			player = m_playerManager->GetNextPlayer();
		}
	}

	// Check if the user wants to exit back to the menu.
	if( g_engine->GetInput()->GetKeyPress( DIK_ESCAPE ) )
	{
		SFProtobufPacket<SFPacketStore::LeaveRoom> leaveRoom(CGSF::LeaveRoom);
		leaveRoom.SetOwnerSerial(g_engine->GetLocalID());

		g_pCasualGameManager->GetNetwork()->TCPSend(&leaveRoom);
	}

	g_pCasualGameManager->Update();
}

//-----------------------------------------------------------------------------
// Render the game state.
//-----------------------------------------------------------------------------
void Game::Render(float elapsed)
{
	// Ensure the scene is loaded.
	if( g_engine->GetSceneManager()->IsLoaded() == false )
		return;

	// If the user is holding down the tab key, then render the score board.
	if( g_engine->GetInput()->GetKeyPress( DIK_TAB, true ) == true )
	{
		m_scoreBoardFont->Render( m_scoreBoardNames, 20, 100, 0xFFFF7700 );
		m_scoreBoardFont->Render( m_scoreBoardFrags, 180, 100, 0xFFFF7700 );
		m_scoreBoardFont->Render( m_scoreBoardDeaths, 260, 100, 0xFFFF7700 );
	}

	// Draw the local player's crosshair in the centre of the screen.
	g_engine->GetSprite()->Begin( D3DXSPRITE_ALPHABLEND );
	g_engine->GetSprite()->Draw( m_crosshair->GetTexture(), NULL, NULL, &D3DXVECTOR3( g_engine->GetDisplayMode()->Width / 2.0f - 15.0f, g_engine->GetDisplayMode()->Height / 2.0f - 15.0f, 0.0f ), 0xFFFFFFFF );
	g_engine->GetSprite()->End();
}

//-----------------------------------------------------------------------------
// Returns a pointer to the bullet manager.
//-----------------------------------------------------------------------------
BulletManager *Game::GetBulletManager()
{
	return m_bulletManager;
}

#define SF_GETPACKET_ARG(a,b,c) memcpy(a,b.c_str(), sizeof(c));

using namespace google;
void Game::HandleNetworkMessage(BasePacket* pPacket)
{
	// Process the received messaged based on its type.
	switch( pPacket->GetPacketID() )
	{
	case CGSF::PeerList:
		{
			SFProtobufPacket<SFPacketStore::PeerList>* pPeerList = (SFProtobufPacket<SFPacketStore::PeerList>*)pPacket;

			for(int i=0; i < pPeerList->GetData().peer_size(); i++)
			{
				const SFPacketStore::PeerList::PeerInfo& Peer = pPeerList->GetData().peer(i);
				_PeerInfo PeerInfo;
				SF_GETPACKET_ARG(&PeerInfo, Peer.info(), _PeerInfo);
				g_pCasualGameManager->GetNetwork()->AddPeer(Peer.serial(), PeerInfo.ExternalIP, PeerInfo.ExternalPort, PeerInfo.LocalIP, PeerInfo.LocalPort);
			}		
		}
		break;
	case CGSF::AddPeer:
		{
			//g_engine->TCPSend(CGSF::PlayerIP, pPacket->GetDataBuffer(), sizeof(PlayerIPMsg));
		}
		break;
	case CGSF::DeletePeer:
		{
			SFProtobufPacket<SFPacketStore::DELETE_PEER>* pDeletePeer = (SFProtobufPacket<SFPacketStore::DELETE_PEER>*)pPacket;
			g_pCasualGameManager->GetNetwork()->DeletePeer(pDeletePeer->GetData().serial());		
		}
		break;

	case CGSF::MSG_CREATE_PLAYER:
		{
			SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>* pMsgCreatePlayer = (SFProtobufPacket<SFPacketStore::MSG_CREATE_PLAYER>*)pPacket;

			PlayerInfo Info;
			Info.PlayerID =  pMsgCreatePlayer->GetData().serial();

			PlayerObject *object = m_playerManager->AddPlayer(&Info);

			if(Info.PlayerID == g_pCasualGameManager->GetMyID())
			{
				m_playerManager->SetLocalPlayer(object);
				g_engine->SetPlayerID(Info.PlayerID);
				g_engine->SetLocalID(Info.PlayerID);
			}
	
			g_engine->GetSceneManager()->AddObject( object );

			if(Info.PlayerID == g_pCasualGameManager->GetMyID())
			{
				SpawnPlayerMsg spm;
				spm.PlayerID = g_engine->GetPlayerID();
				spm.translation = g_engine->GetSceneManager()->GetSpawnPointByID( pMsgCreatePlayer->GetData().spawnindex() )->GetTranslation();

				SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER> request(CGSF::MSG_SPAWN_PLAYER);
				request.GetData().set_spawnplayer(&spm, sizeof(SpawnPlayerMsg));
				g_pCasualGameManager->GetNetwork()->TCPSend(&request);
			}
			

			//int spawnid = g_engine->GetSceneManager()->GetSpawnPointID( g_engine->GetSceneManager()->GetRandomPlayerSpawnPoint() );
			//m_playerManager->SpawnLocalPlayer( spawnid );
			//m_playerManager->SpawnPlayer( Info.PlayerID , g_engine->GetSceneManager()->GetSpawnPointByID(PktMsgCreatePlayer.spawnindex() )->GetTranslation());
		}
		break;

	case CGSF::MSG_DESTROY_PLAYER:
		{
			SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>* pMsgDestroyPlayer = (SFProtobufPacket<SFPacketStore::MSG_DESTROY_PLAYER>*)pPacket;

			// Remove the player from the player manager and the scene.
			SceneObject *object = m_playerManager->GetPlayer( pMsgDestroyPlayer->GetData().serial() );
			g_engine->GetSceneManager()->RemoveObject( &object );
			m_playerManager->RemovePlayer( pMsgDestroyPlayer->GetData().serial() );
		}
		break;

	case CGSF::MSG_SPAWN_PLAYER:
		{
			SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER>* pPlayerSpawn = (SFProtobufPacket<SFPacketStore::MSG_SPAWN_PLAYER>*)pPacket;

			// Get a pointer to the game specific network message.
			SpawnPlayerMsg msg;
			SF_GETPACKET_ARG(&msg, pPlayerSpawn->GetData().spawnplayer(), SpawnPlayerMsg);

			// Spawn the player.
			m_playerManager->SpawnPlayer( msg.PlayerID, msg.translation );
			
			PlayerObject *player = m_playerManager->GetPlayer( msg.PlayerID );


			if(player != NULL && msg.PlayerID == g_engine->GetPlayerID())
			{
				m_playerManager->GetLocalPlayer()->SetEnabled(true);

			}
		}
		break;
/*
	case CGSF::MSG_TERMINATE_SESSION:
		{
			// Switch to the menu state.
			g_engine->ChangeState( STATE_MENU );

			break;
		}
*/

	

	case  CGSF::MSG_PLAYER_HEALTH:
		{
			SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH>* pPlayerHealth = (SFProtobufPacket<SFPacketStore::MSG_PLAYER_HEALTH>*)pPacket;
			
			PlayerHealthMsg msg;
			SF_GETPACKET_ARG(&msg, pPlayerHealth->GetData().playerhealth(), PlayerHealthMsg);
			// Set the player's health.
			m_playerManager->GetPlayer( msg.PlayerID )->SetHealth( msg.health );

			// Check if the player has been killed.
			if( msg.health <= 0.0f )
				m_playerManager->GetPlayer( msg.PlayerID )->Kill();
		}
		break;
	case CGSF::MSG_PLAYER_SCORE:
		{
			SFProtobufPacket<SFPacketStore::MSG_PLAYER_SCORE>* pPlayerScore = (SFProtobufPacket<SFPacketStore::MSG_PLAYER_SCORE>*)pPacket;

			PlayerScoreMsg msg;
			SF_GETPACKET_ARG(&msg, pPlayerScore->GetData().playerscore(), PlayerScoreMsg);

			// Update the player's score.
			m_playerManager->GetPlayer( msg.PlayerID )->SetFrags( msg.frags );
			m_playerManager->GetPlayer( msg.PlayerID )->SetDeaths( msg.deaths );
		}
		break;

	case CGSF::MSG_PLAYER_WEAPON_CHANGE:
		{
			SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGE>* pPlayerWeaponChange = (SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGE>*)pPacket;

			PlayerWeaponChangeMsg msg;
			SF_GETPACKET_ARG(&msg, pPlayerWeaponChange->GetData().weaponchange(), PlayerWeaponChangeMsg);

			// Change the player's weapon.
			m_playerManager->GetPlayer( msg.PlayerID )->WeaponChanged( msg.weapon );
		}
		break;

	case CGSF::MSG_PLAYER_WEAPON_CHANGING:
		{
			SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGING>* pWeaponChanging = (SFProtobufPacket<SFPacketStore::MSG_PLAYER_WEAPON_CHANGING>*)pPacket;

			PlayerWeaponChangeMsg msg;
			SF_GETPACKET_ARG(&msg, pWeaponChanging->GetData().weaponchanging(), PlayerWeaponChangeMsg);

			// Indicate that this player is changing weapons.
			m_playerManager->GetPlayer( msg.PlayerID )->WeaponChanging();		
		}	
		break;

	}
}

void Game::HandleNetworkMessage( NetworkMessage* pMessage )
{
	switch(pMessage->msgid)
	{
	case CGSF::MSG_PLAYER_MOVE_UPDATE:
		{
			// Get a pointer to the game specific network message.
			PlayerMoveUpdateMsg *msg = (PlayerMoveUpdateMsg *)pMessage;

			// Get a pointer to the player in question.
			PlayerObject *player = m_playerManager->GetPlayer( msg->PlayerID );

			if(player == NULL)
				break;

			// If the player is disabled then chances are the local player has
			// just joined the game, and is receiving messages about a player
			// that it does not realise has already spawned. So just enable
			// that player and start updating it.
			if( player->GetEnabled() == false )
			{
				player->SetEnabled( true );
				player->SetVisible( true );
			}

			// Update all the player's details.
			player->SetTranslation( msg->translation.x, msg->translation.y, msg->translation.z );
			player->SetDrive( msg->drive );
			player->SetStrafe( msg->strafe );
			player->SetFire( msg->fire );


		}
		break;

	case CGSF::MSG_PLAYER_LOOK_UPDATE:
		{
		
			// Get a pointer to the game specific network message.
			PlayerLookUpdateMsg* msg = (PlayerLookUpdateMsg *)pMessage;;


			// Update the player's rotation and view tilt.

			if(m_playerManager->GetPlayer( msg->PlayerID ))
			{
				m_playerManager->GetPlayer( msg->PlayerID )->SetRotation( 0.0f, msg->rotationY, 0.0f );
				m_playerManager->GetPlayer( msg->PlayerID )->SetViewTilt( msg->viewTilt );
			}
		}

		break;
	}
}