#pragma once

class SFFastCRC
{
public:
	SFFastCRC(void);
	virtual ~SFFastCRC(void);

	BOOL Initialize();
	BOOL GetCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	BOOL GetSimpleCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	BOOL Get7zCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	BOOL GetZLibCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	BOOL GetadlerCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
};



