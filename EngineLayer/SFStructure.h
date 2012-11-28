#pragma once
//#include <d3dx9math.h>
#pragma pack(push, 1)

typedef struct tag_SFPacketHeader
{
	USHORT PacketLen;
	DWORD  PacketOption;
	DWORD  DataCRC;

	tag_SFPacketHeader()
	{
		Clear();
	}

	inline BOOL CheckDataCRC(){return (PacketOption & PACKET_OPTION_DATACRC) != 0;}
	inline BOOL CheckEncryption(){return (PacketOption & PACKET_OPTION_ENCRYPTION) != 0;}
	inline BOOL CheckCompressed(){return (PacketOption & PACKET_OPTION_COMPRESS) != 0;}
	inline void SetPacketOption(DWORD Option){ PacketOption = Option;}

	void Clear()
	{
		PacketLen = 0;
		PacketOption = 0;
		DataCRC = 0;
	}
}SFPacketHeader;

typedef struct tag_DBConnectionInfo
{
	CHAR szServiceName[100]; 
	CHAR szDBName[100];
	CHAR szID[100];
	CHAR szPassword[100];
	CHAR IP[100];
	USHORT Port;
}_DBConnectionInfo;

typedef struct tag_TimerInfo
{
	DWORD TimerID;
	DWORD Period;
	USHORT StartDelay;
	
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

#pragma pack(pop)