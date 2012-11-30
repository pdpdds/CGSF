#include "stdafx.h"
#include "7zcrc.h"



#define kCrcPoly 0xEDB88320
#define CRC_NUM_TABLES 8
static UINT32 g_CrcTable[256 * CRC_NUM_TABLES];



void CrcGenerateTable()
{
	UINT32 i;
	for (i = 0; i < 256; i++)
	{
		UINT32 r = i;
		unsigned j;
		for (j = 0; j < 8; j++)
			r = (r >> 1) ^ (kCrcPoly & ~((r & 1) - 1));
		g_CrcTable[i] = r;
	}

	for (; i < 256 * CRC_NUM_TABLES; i++)
	{
		UINT32 r = g_CrcTable[i - 256];
		g_CrcTable[i] = g_CrcTable[r & 0xFF] ^ (r >> 8);
	}
}



DWORD crc7z(DWORD crc, BYTE* data, int len)
{
	crc = crc ^ 0xffffffffL;

	crc = CrcUpdateT8(crc, data, len, g_CrcTable);

	return crc ^ 0xffffffffL;
};


