#pragma once 

namespace ServerLib { 

	class PktHandlerLobby
	{
	public:
		PktHandlerLobby();
		~PktHandlerLobby();

		short RequestLeaveLobby(const WorkerPacket& requestPacket);
	};
}