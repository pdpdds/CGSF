#pragma once

class SFChecksum
{
public:
	static DWORD FromBuffer(BYTE* buffer, size_t size, DWORD &dwCrc32);
	static DWORD FromFileWin32(LPCWSTR filename, DWORD &dwCrc32);
	static DWORD FromFileFilemap(LPCWSTR filename, DWORD &dwCrc32);
};
