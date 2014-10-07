#include "stdafx.h"

#include "ServerErrorCode.h"
#include "WorkerPacket.h"
#include "PktHandlerLobby.h"

namespace ServerLib { 

	PktHandlerLobby::PktHandlerLobby()
	{

	}

	PktHandlerLobby::~PktHandlerLobby()
	{

	}

	short PktHandlerLobby::RequestLeaveLobby(const WorkerPacket& requestPacket)
	{
		UNREFERENCED_PARAMETER(requestPacket);

		return SERVER_ERROR_CODE::NONE;
	}
}