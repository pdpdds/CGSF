#pragma once

class SFFastCRC
{
public:
	SFFastCRC(void);
	virtual ~SFFastCRC(void);

	bool Initialize();
	bool GetCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	bool GetSimpleCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	bool Get7zCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	bool GetZLibCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
	bool GetadlerCRC(BYTE* pBuffer, DWORD BufferSize, DWORD& CRC);
};



