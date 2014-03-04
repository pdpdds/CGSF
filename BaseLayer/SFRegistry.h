#pragma once
#include <winreg.h>
#include <Shlwapi.h>
#include <windows.h>
#include <string>

class SFRegistry
{
public:
    SFRegistry(HKEY hRootKey);
	virtual ~SFRegistry();

	BOOL CreateRegistryKey(const TCHAR* szSubKey);
	BOOL OpenRegistryKey(const  TCHAR* szSubKey);
	void CloseRegistryKey();
	BOOL DeleteKey(const TCHAR* szSubKey );

	BOOL SetValue(const  TCHAR* szChildKey, DWORD dwValue);
    BOOL SetValue(const  TCHAR* szChildKey, const TCHAR* szChildKeyValue);

	BOOL GetValue(const  TCHAR* szChildKey, LPCTSTR szChildValue, DWORD& dwLen);
	BOOL GetValue(const  TCHAR* szChildKey, DWORD& dwValue);

	BOOL DeleteValue(const  TCHAR* szChildKey);

protected:

private:
	HKEY m_hRootKey;
	HKEY m_hRegKey;
};
