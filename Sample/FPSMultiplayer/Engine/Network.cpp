//-----------------------------------------------------------------------------
// Network.h implementation.
// Refer to the Network.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// The network class constructor.
//-----------------------------------------------------------------------------
/*Network::Network( GUID guid, void (*HandleNetworkMessageFunction)( ReceivedMessage *msg ) )
{
	// Initialise the critical sections.
	InitializeCriticalSection( &m_sessionCS );
	InitializeCriticalSection( &m_playerCS );
	InitializeCriticalSection( &m_messageCS );

	// Invalidate the DirectPlay peer interface and the device.
	m_dpp = NULL;
	m_device = NULL;

	// Store the application's GUID.
	memcpy( &m_guid, &guid, sizeof( GUID ) );

	// Create the enumerated session list.
	m_sessions = new LinkedList< SessionInfo >;

	// Create the player list.
	m_players = new LinkedList< PlayerInfo >;

	// Create the network message list.
	m_messages = new LinkedList< ReceivedMessage >;

	// Load the network settings.
	Script *settings = new Script( "NetworkSettings.txt" );
	if( settings->GetNumberData( "processing_time" ) == NULL )
	{
		m_port = 2509;
		m_sendTimeOut = 100;
		m_processingTime = 100;
	}
	else
	{
		m_port = *settings->GetNumberData( "port" );
		m_sendTimeOut = *settings->GetNumberData( "send_time_out" );
		m_processingTime = *settings->GetNumberData( "processing_time" );
	}
	SAFE_DELETE( settings );

	// Initially the network is not allowed to receive application specific messages.
	m_receiveAllowed = false;

	// Set the network message handler.
	HandleNetworkMessage = HandleNetworkMessageFunction;

	// Create and initialise the DirectPlay peer interface.
	CoCreateInstance( CLSID_DirectPlay8Peer, NULL, CLSCTX_INPROC, IID_IDirectPlay8Peer, (void**)&m_dpp );
	m_dpp->Initialize( (PVOID)this, NetworkMessageHandler, DPNINITIALIZE_HINT_LANSESSION );

	// Create the device address.
	CoCreateInstance( CLSID_DirectPlay8Address, NULL, CLSCTX_INPROC, IID_IDirectPlay8Address, (LPVOID*) &m_device );
	m_device->SetSP( &CLSID_DP8SP_TCPIP );
	m_device->AddComponent( DPNA_KEY_PORT, &m_port, sizeof(DWORD), DPNA_DATATYPE_DWORD );
}

//-----------------------------------------------------------------------------
// The network class destructor.
//-----------------------------------------------------------------------------
Network::~Network()
{
	// Save the network settings.
	Script *settings = new Script( "NetworkSettings.txt" );
	if( settings->GetNumberData( "processing_time" ) == NULL )
	{
		settings->AddVariable( "port", VARIABLE_NUMBER, &m_port );
		settings->AddVariable( "send_time_out", VARIABLE_NUMBER, &m_sendTimeOut );
		settings->AddVariable( "processing_time", VARIABLE_NUMBER, &m_processingTime );
	}
	else
	{
		settings->SetVariable( "port", &m_port );
		settings->SetVariable( "send_time_out", &m_sendTimeOut );
		settings->SetVariable( "processing_time", &m_processingTime );
	}
	settings->SaveScript();
	SAFE_DELETE( settings );

	// Release the device address.
	SAFE_RELEASE( m_device );

	// Close and release the DirectPlay peer interface.
	if( m_dpp != NULL )
		m_dpp->Close( DPNCLOSE_IMMEDIATE );
	SAFE_RELEASE( m_dpp );

	// Destroy the enumerated session list.
	SAFE_DELETE( m_sessions );

	// Destroy the player list.
	SAFE_DELETE( m_players );

	// Destroy the network message list.
	SAFE_DELETE( m_messages );

	// Delete the critical sections.
	DeleteCriticalSection( &m_sessionCS );
	DeleteCriticalSection( &m_playerCS );
	DeleteCriticalSection( &m_messageCS );
}

//-----------------------------------------------------------------------------
// Updates the network object, allowing it to process messages.
//-----------------------------------------------------------------------------
void Network::Update()
{
	EnterCriticalSection( &m_messageCS );

	ReceivedMessage *message = m_messages->GetFirst();

	unsigned long endTime = timeGetTime() + m_processingTime;
	while( endTime > timeGetTime() && message != NULL )
	{
		HandleNetworkMessage( message );
		m_messages->Remove( &message );
		message = m_messages->GetFirst();
	}

	LeaveCriticalSection( &m_messageCS );
}

//-----------------------------------------------------------------------------
// Enumerates for sessions on the local network.
//-----------------------------------------------------------------------------
void Network::EnumerateSessions()
{
	// Empty the lists.
	m_players->Empty();
	m_messages->Empty();
	m_sessions->Empty();

	// Prepare the application description.
	DPN_APPLICATION_DESC description;
	ZeroMemory( &description, sizeof( DPN_APPLICATION_DESC ) );
	description.dwSize = sizeof( DPN_APPLICATION_DESC );
	description.guidApplication = m_guid;

	// Enumerate sessions synchronously.
	m_dpp->EnumHosts( &description, NULL, m_device, NULL, 0, 1, 0, 0, NULL, NULL, DPNENUMHOSTS_SYNC );
}

//-----------------------------------------------------------------------------
// Attempts to host a session.
//-----------------------------------------------------------------------------
bool Network::Host( char *name, char *session, int players, void *playerData, unsigned long dataSize )
{
	WCHAR wide[MAX_PATH];

	// Prepare and set the player information structure.
	DPN_PLAYER_INFO player;
	ZeroMemory( &player, sizeof( DPN_PLAYER_INFO ) );
	player.dwSize = sizeof( DPN_PLAYER_INFO );
	player.pvData = playerData;
	player.dwDataSize = dataSize;
	player.dwInfoFlags = DPNINFO_NAME | DPNINFO_DATA;
	mbstowcs( wide, name, MAX_PATH );
	player.pwszName = wide;
	if( FAILED( m_dpp->SetPeerInfo( &player, NULL, NULL, DPNSETPEERINFO_SYNC ) ) )
		return false;

	// Prepare the application description.
	DPN_APPLICATION_DESC description;
	ZeroMemory( &description, sizeof( DPN_APPLICATION_DESC ) );
	description.dwSize = sizeof( DPN_APPLICATION_DESC );
	description.guidApplication = m_guid;
	description.dwMaxPlayers = players;
	mbstowcs( wide, session, MAX_PATH );
	description.pwszSessionName = wide;

	// Host the session.
	if( FAILED( m_dpp->Host( &description, &m_device, 1, NULL, NULL, NULL, 0 ) ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Attempts to join a selected session from the enumerated session list.
//-----------------------------------------------------------------------------
bool Network::Join( char *name, int session, void *playerData, unsigned long dataSize )
{
	WCHAR wide[MAX_PATH];

	// Empty the player list and the newtork message.
	m_players->Empty();
	m_messages->Empty();

	// Ignore invalid sessions.
	if( session < 0 )
		return false;

	// Prepare and set the player information structure.
	DPN_PLAYER_INFO player;
	ZeroMemory( &player, sizeof( DPN_PLAYER_INFO ) );
	player.dwSize = sizeof( DPN_PLAYER_INFO );
	player.pvData = playerData;
	player.dwDataSize = dataSize;
	player.dwInfoFlags = DPNINFO_NAME | DPNINFO_DATA;
	mbstowcs( wide, name, MAX_PATH );
	player.pwszName = wide;
	if( FAILED( m_dpp->SetPeerInfo( &player, NULL, NULL, DPNSETPEERINFO_SYNC ) ) )
		return false;

	// Enter the sessions linked list critical section.
	EnterCriticalSection( &m_sessionCS );

	// Find the host of the selected session.
	m_sessions->Iterate( true );
	for( int s = 0; s < session + 1; s++ )
	{
		if( m_sessions->Iterate() ==  NULL )
		{
			LeaveCriticalSection( &m_sessionCS );
			return false;
		}
	}

	// Join the session.
	if( FAILED( m_dpp->Connect( &m_sessions->GetCurrent()->description, m_sessions->GetCurrent()->address, m_device, NULL, NULL, NULL, 0, NULL, NULL, NULL, DPNCONNECT_SYNC ) ) )
	{
		LeaveCriticalSection( &m_sessionCS );
		return false;
	}
	LeaveCriticalSection( &m_sessionCS );

	return true;
}

//-----------------------------------------------------------------------------
// Terminates the current session.
//-----------------------------------------------------------------------------
void Network::Terminate()
{
	// Only allow the host to terminate a session.
	if( m_dpnidHost == m_dpnidLocal )
		m_dpp->TerminateSession( NULL, 0, 0 );

	// Close the connection. This will also uninitialise the DirectPlay peer interface.
	if( m_dpp != NULL )
		m_dpp->Close( DPNCLOSE_IMMEDIATE );

	// Initialise the DirectPlay peer interface.
	m_dpp->Initialize( (PVOID)this, NetworkMessageHandler, DPNINITIALIZE_HINT_LANSESSION );
}

//-----------------------------------------------------------------------------
// Sets the receive allowed flag.
//-----------------------------------------------------------------------------
void Network::SetReceiveAllowed( bool allowed )
{
	m_receiveAllowed = allowed;
}

//-----------------------------------------------------------------------------
// Returns the next iterated session from the enumerated session list.
//-----------------------------------------------------------------------------
SessionInfo *Network::GetNextSession( bool restart )
{
	EnterCriticalSection( &m_sessionCS );

	m_sessions->Iterate( restart );
	if( restart == true )
		m_sessions->Iterate();

	LeaveCriticalSection( &m_sessionCS );

	return m_sessions->GetCurrent();
}

//-----------------------------------------------------------------------------
// Returns a pointer to the player information structure of the given player.
//-----------------------------------------------------------------------------
PlayerInfo *Network::GetPlayer( DPNID dpnid )
{
	EnterCriticalSection( &m_playerCS );

	m_players->Iterate( true );
	while( m_players->Iterate() )
	{
		if( m_players->GetCurrent()->dpnid == dpnid )
		{
			LeaveCriticalSection( &m_playerCS );

			return m_players->GetCurrent();
		}
	}

	LeaveCriticalSection( &m_playerCS );

	return NULL;
}

//-----------------------------------------------------------------------------
// Returns the local player's DirectPlay ID.
//-----------------------------------------------------------------------------
DPNID Network::GetLocalID()
{
	return m_dpnidLocal;
}

//-----------------------------------------------------------------------------
// Returns the host's DirectPlay ID.
//-----------------------------------------------------------------------------
DPNID Network::GetHostID()
{
	return m_dpnidHost;
}

//-----------------------------------------------------------------------------
// Indicates if the current network object is hosting or not.
//-----------------------------------------------------------------------------
bool Network::IsHost()
{
	if( m_dpnidHost == m_dpnidLocal )
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
// Sends a network message.
//-----------------------------------------------------------------------------
void Network::Send( void *data, long size, DPNID dpnid, long flags )
{
	DPNHANDLE hAsync;
	DPN_BUFFER_DESC dpbd;

	if( ( dpbd.dwBufferSize = size ) == 0 )
		return;
	dpbd.pBufferData = (BYTE*)data;

	m_dpp->SendTo( dpnid, &dpbd, 1, m_sendTimeOut, NULL, &hAsync, flags | DPNSEND_NOCOMPLETE | DPNSEND_COALESCE );
}

//-----------------------------------------------------------------------------
// The internal network message handler.
//-----------------------------------------------------------------------------
HRESULT WINAPI Network::NetworkMessageHandler( PVOID context, DWORD msgid, PVOID data )
{
	// Get a pointer to the calling network object.
	Network *network = (Network*)context;

	// Process the incoming message based on its type.
	switch( msgid )
	{
		case DPN_MSGID_CREATE_PLAYER:
		{
			unsigned long size = 0;
			DPN_PLAYER_INFO *info = NULL;
			HRESULT hr = DPNERR_CONNECTING;
			PDPNMSG_CREATE_PLAYER msgCreatePlayer = (PDPNMSG_CREATE_PLAYER)data;

			// Create a player information structure for the new player.
			PlayerInfo *playerInfo = new PlayerInfo;
			ZeroMemory( playerInfo, sizeof( PlayerInfo ) );
			playerInfo->dpnid = msgCreatePlayer->dpnidPlayer;

			// Keep calling GetPeerInfo(), as it may still be trying to connect.
			while( hr == DPNERR_CONNECTING )
				hr = network->m_dpp->GetPeerInfo( playerInfo->dpnid, info, &size, 0 );

			// Check if GetPeerInfo() has returned the size of the DPN_PLAYER_INFO structure.
			if( hr == DPNERR_BUFFERTOOSMALL )
			{
				info = (DPN_PLAYER_INFO*) new BYTE[size];
				ZeroMemory( info, size );
				info->dwSize = sizeof( DPN_PLAYER_INFO );

				// Try again using the correct size.
				if( SUCCEEDED( network->m_dpp->GetPeerInfo( playerInfo->dpnid, info, &size, 0 ) ) )
				{
					// Store the name of the new player.
					playerInfo->name = new char[wcslen( info->pwszName ) + 1];
					ZeroMemory( playerInfo->name, wcslen( info->pwszName ) + 1 );
					wcstombs( playerInfo->name, info->pwszName, wcslen( info->pwszName ) );

					// Store the player data.
					playerInfo->data = new BYTE[info->dwDataSize];
					memcpy( playerInfo->data, info->pvData, info->dwDataSize );
					playerInfo->size = info->dwDataSize;

					// Store any local and host player details.
					if( info->dwPlayerFlags & DPNPLAYER_LOCAL )
						network->m_dpnidLocal = playerInfo->dpnid;
					if( info->dwPlayerFlags & DPNPLAYER_HOST )
						network->m_dpnidHost = playerInfo->dpnid;
				}

				SAFE_DELETE_ARRAY( info );
			}

			// Add the new player to the player list.
			EnterCriticalSection( &network->m_playerCS );
			network->m_players->Add( playerInfo );
			LeaveCriticalSection( &network->m_playerCS );

			// If there is no network message handler, then break now.
			if( network->HandleNetworkMessage == NULL )
				break;

			// Create a create player message.
			ReceivedMessage *message = new ReceivedMessage;
			message->msgid = MSGID_CREATE_PLAYER;
			message->dpnid = playerInfo->dpnid;

			// Store the message so that it can be processed by the application later.
			EnterCriticalSection( &network->m_messageCS );
			network->m_messages->Add( message );
			LeaveCriticalSection( &network->m_messageCS );

			break;
		}

		case DPN_MSGID_DESTROY_PLAYER:
		{
			// Find the player to destroy and remove it from the player list.
			EnterCriticalSection( &network->m_playerCS );
			network->m_players->Iterate( true );
			while( network->m_players->Iterate() )
			{
				if( network->m_players->GetCurrent()->dpnid == ( (PDPNMSG_DESTROY_PLAYER)data )->dpnidPlayer )
				{
					network->m_players->Remove( (PlayerInfo**)network->m_players->GetCurrent() );
					break;
				}
			}
			LeaveCriticalSection( &network->m_playerCS );

			// If there is no network message handler, then break now.
			if( network->HandleNetworkMessage == NULL )
				break;

			// Create a destroy player message.
			ReceivedMessage *message = new ReceivedMessage;
			message->msgid = MSGID_DESTROY_PLAYER;
			message->dpnid = ( (PDPNMSG_DESTROY_PLAYER)data )->dpnidPlayer;

			// Store the message so that it can be processed by the application later.
			EnterCriticalSection( &network->m_messageCS );
			network->m_messages->Add( message );
			LeaveCriticalSection( &network->m_messageCS );

			break;
		}

		case DPN_MSGID_ENUM_HOSTS_RESPONSE:
		{
			PDPNMSG_ENUM_HOSTS_RESPONSE response = (PDPNMSG_ENUM_HOSTS_RESPONSE)data;

			// Create a session information structure for the new session.
			SessionInfo *sessionInfo = new SessionInfo;
			response->pAddressSender->Duplicate( &sessionInfo->address );
			memcpy( &sessionInfo->description, response->pApplicationDescription, sizeof( DPN_APPLICATION_DESC ) );

			// Add the new session to the session list.
			EnterCriticalSection( &network->m_sessionCS );
			network->m_sessions->Add( sessionInfo );
			LeaveCriticalSection( &network->m_sessionCS );

			break;
		}

		case DPN_MSGID_RECEIVE:
		{
			// If there is no network message handler, then break now.
			if( network->HandleNetworkMessage == NULL )
				break;

			// Check if the network is allowed to receive application specific messages.
			if( network->m_receiveAllowed == false )
				break;

			// Create a receive message.
			ReceivedMessage *message = new ReceivedMessage;
			memcpy( message, ( (PDPNMSG_RECEIVE)data )->pReceiveData, ( (PDPNMSG_RECEIVE)data )->dwReceiveDataSize );

			// Store the message so that it can be processed by the application later.
			EnterCriticalSection( &network->m_messageCS );
			network->m_messages->Add( message );
			LeaveCriticalSection( &network->m_messageCS );

			break;
		}

		case DPN_MSGID_TERMINATE_SESSION:
		{
			// If there is no network message handler, then break now.
			if( network->HandleNetworkMessage == NULL )
				break;

			// Create a terminate session message.
			ReceivedMessage *message = new ReceivedMessage;
			message->msgid = MSGID_TERMINATE_SESSION;

			// Store the message so that it can be processed by the application later.
			EnterCriticalSection( &network->m_messageCS );
			network->m_messages->Add( message );
			LeaveCriticalSection( &network->m_messageCS );

			break;
		}
	}

	return S_OK;
}*/