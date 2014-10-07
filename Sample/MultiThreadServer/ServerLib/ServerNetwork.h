#pragma once


namespace ServerLib { 
	class PacketDispatcher;
	class PacketDistribute;

	class ServerNetwork
	{
	public:
		ServerNetwork(void);
		virtual ~ServerNetwork(void);


		NET_ERROR_CODE Init(int dispatchThreadCount, int maxBufferSize, int maxPacketSize, int packetOption);
		
		bool Start();

		void Stop();
		
				

	private:
		std::unique_ptr<PacketDispatcher> m_pPacketDispatcher;
		std::unique_ptr<PacketDistribute> m_pPacketDistribute;
	};
}