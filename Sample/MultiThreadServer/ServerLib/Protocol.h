#pragma once 


namespace ServerLib {

	enum class PACKET_ID : unsigned short
	{
		NONE				= 0,

		SYSTEM_CONNECT		= 901,
		SYSTEM_DISCONNECT	= 902,
		
		REQUEST_LOGIN		= 1001,
	};
}