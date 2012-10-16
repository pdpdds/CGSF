#pragma once

#include <dbghelp.h>

enum BasicType  // Stolen from CVCONST.H in the DIA 2.0 SDK
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

typedef LONG (WINAPI *PCERUserFunc)( HANDLE hFile );
typedef PCERUserFunc LPCERUserFunc;

class CExceptionReport
{
public:
	CExceptionReport();
	~CExceptionReport();

    static void SetProgramName( PTSTR pszProgamName = NULL );
	static void SetUserFunc( LPCERUserFunc lpUserFunc );
	
	static LONG WINAPI UnhandledExceptionFilter( PEXCEPTION_POINTERS pExceptionInfo );

    static int __cdecl _tprintf( const TCHAR * format, ... );

private:
	static void WriteBasicInfo( PEXCEPTION_RECORD pExceptionRecord );
	static void WriteExceptionReport( PEXCEPTION_POINTERS pExceptionInfo );
    static void WriteStackDetails( PCONTEXT pContext, BOOL bWriteVariables );
    static void WriteRegistersInfo( PCONTEXT pContext );
	static void WriteMemoryDump( PCONTEXT pContext );

	static void StoreCoreDump( void );
	static void Dump( DWORD64 pData, DWORD dwSize, BOOL bAlign );

    static LPTSTR GetExceptionString( DWORD dwCode );
    static BOOL GetLogicalAddress(  PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset );
    static BOOL CALLBACK EnumerateSymbolsCallback(PSYMBOL_INFO,ULONG, PVOID);
    static BOOL FormatSymbolValue( PSYMBOL_INFO, STACKFRAME *, char * pszBuffer, unsigned cbBuffer );
    static char * DumpTypeIndex( char *, DWORD64, DWORD, unsigned, DWORD_PTR, BOOL & );
    static char * FormatOutputValue( char * pszCurrBuffer, BasicType basicType, DWORD64 length, PVOID pAddress );
    static BasicType GetBasicType( DWORD typeIndex, DWORD64 modBase );

	static LPCERUserFunc m_lpUserFunc;
	static LPTOP_LEVEL_EXCEPTION_FILTER m_OldFilter;
	static TCHAR m_szLogPrefixName[ MAX_PATH ];
	static TCHAR m_szModuleName[ MAX_PATH ];
	static HANDLE m_hReportFile;
	static HANDLE m_hProcess;
	static BOOL m_bHasSymbol;

public:
	static int bSF;
};

//extern CExceptionReport g_CExceptionReport;
