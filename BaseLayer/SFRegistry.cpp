#include "StdAfx.h"
#include "SFRegistry.h"
#include <tchar.h>

SFRegistry::SFRegistry(HKEY hRootKey)
{
	m_hRootKey = hRootKey;
	m_hRegKey = NULL;
}


SFRegistry::~SFRegistry()
{
	CloseRegistryKey();	
}

void SFRegistry::CloseRegistryKey()
{
	if(m_hRegKey)
		RegCloseKey(m_hRegKey);

	m_hRegKey = NULL;
}

BOOL SFRegistry::DeleteKey(const  TCHAR* szSubKey )
{
	CloseRegistryKey();
			
	if( ERROR_SUCCESS  != RegDeleteKey( m_hRootKey, szSubKey ))
	{ 	
			return false;
	}
	
	return true;
}

BOOL SFRegistry::CreateRegistryKey(const  TCHAR* szSubKey)
{
	DWORD dwDisp = 0;

    if( ERROR_SUCCESS  != RegCreateKeyEx( m_hRootKey, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
					                          KEY_ALL_ACCESS, NULL, (PHKEY)&m_hRegKey,  &dwDisp))
	{
        return FALSE;
    }

	return TRUE;
}

BOOL SFRegistry::OpenRegistryKey(const  TCHAR* szSubKey)
{
	CloseRegistryKey();

	if (RegOpenKey(m_hRootKey, szSubKey, &m_hRegKey) != ERROR_SUCCESS)
		return FALSE;	

	return TRUE;
}

BOOL SFRegistry::SetValue(const  TCHAR* szChildKey, const TCHAR* szChildKeyValue )
{
	if(NULL == m_hRegKey)
		return false;

	TCHAR szValue[150];
	_tcscpy_s( szValue, (const wchar_t *)szChildKeyValue );
	if( ERROR_SUCCESS  != RegSetValueEx(m_hRegKey, szChildKey, 0, REG_SZ, 
		(BYTE* const)&szValue, ((DWORD)(_tcslen(szValue) + 1)) * sizeof(TCHAR)))
	{		
		return false;
	}	
	return true;
}

BOOL SFRegistry::SetValue( const  TCHAR* szChildKey, DWORD dwValue )
{
	if(NULL == m_hRegKey)
		return false;

	if (RegSetValueEx(m_hRegKey, szChildKey, 0, REG_DWORD, (BYTE*) &dwValue, sizeof(DWORD))
		!= ERROR_SUCCESS)
		return FALSE;

	return TRUE;

}

BOOL SFRegistry::GetValue(const  TCHAR* szChildKey, LPCTSTR szChildValue, DWORD& dwLen)
{
	if(NULL == m_hRegKey)
		return false;

	DWORD ValueType = 0;	

	if (RegQueryValueEx(m_hRegKey, szChildKey, 0, &ValueType, (BYTE*)szChildValue, &dwLen)
		!= ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL SFRegistry::GetValue(const  TCHAR* szChildKey, DWORD& dwValue)
{
	if(NULL == m_hRegKey)
		return false;

	DWORD	ValueType	= 0;
	DWORD dwLen	= sizeof(DWORD);

	if (RegQueryValueEx(m_hRegKey, szChildKey, 0, &ValueType, (BYTE*) &dwValue, &dwLen) != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

BOOL SFRegistry::DeleteValue(const  TCHAR* szChildKey)
{
	if(NULL == m_hRegKey)
		return false;

	if (RegDeleteValue(m_hRegKey, szChildKey) != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

