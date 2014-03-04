#pragma once
#pragma pack(push, 1)

typedef struct tag_UserInfo
{
	int Serial;
	int ConnectionCount;

	tag_UserInfo()
	{
		Serial = -1;
		ConnectionCount = 0;
	}

}_UserInfo;

typedef struct tag_IPInfo
{
	DWORD LocalIP;
	USHORT LocalPort;
	DWORD ExternalIP;
	USHORT ExternalPort;

	tag_IPInfo()
	{
		Clear();
	}

	void Clear()
	{
		LocalIP = 0;
		LocalPort = 0;
		ExternalIP = 0;
		ExternalPort = 0;
	}

}_IPInfo;

#pragma pack(pop)