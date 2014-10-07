#pragma once 

namespace ServerLib { 

	class WorkerPacket;

	class PktHandlerCommon
	{
	public:
		PktHandlerCommon();
		~PktHandlerCommon();

		short SystemOnConnect(const WorkerPacket& requestPacket);

		short SystemOnDisconnect(const WorkerPacket& requestPacket);

		short RequestLoging(const WorkerPacket& requestPacket);

	};
}