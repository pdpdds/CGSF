#pragma once
//#include <d3dx9math.h>
#pragma pack(push, 1)

typedef struct tag_SFPacketHeader
{
	USHORT packetID;
	DWORD  packetOption;
	DWORD  dataCRC;
	USHORT dataSize;

	tag_SFPacketHeader()
	{
		Clear();
	}

	inline bool CheckDataCRC(){ return (packetOption & PACKET_OPTION_DATACRC) != 0; }
	inline bool CheckEncryption(){ return (packetOption & PACKET_OPTION_ENCRYPTION) != 0; }
	inline bool CheckCompressed(){ return (packetOption & PACKET_OPTION_COMPRESS) != 0; }
	inline void SetPacketOption(DWORD Option){ packetOption = Option; }

	void Clear()
	{
		packetID = 0;
		packetOption = 0;
		dataCRC = 0;
		dataSize = 0;
	}
}SFPacketHeader;

typedef struct tag_TimerInfo
{
	DWORD TimerID;
	DWORD Period;
	DWORD StartDelay;
	
	tag_TimerInfo()
	{
		Clear();
	}

	void Clear()
	{
		TimerID = 0;
		Period = 0;
		StartDelay = 0;
	}

}_TimerInfo;

typedef struct tag_ListenerInfo
{
	int listenerId;
	int packetProtocolId;

	tag_ListenerInfo()
	{
		listenerId = 0;
		packetProtocolId = 0;
	}
}_ListenerInfo;

#pragma pack(pop)

typedef struct tag_ConnectorInfo
{
	std::wstring szIP;
	unsigned short port;
	int connectorId;
	int packetProtocolId;

	std::wstring szDesc;
	bool connected;

	tag_ConnectorInfo()
	{
		port = 0;
		connectorId = 0;
		connected = false;
	}

}_ConnectorInfo;