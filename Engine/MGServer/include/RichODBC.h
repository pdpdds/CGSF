// RichODBC.h: interface for the RichODBC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RichODBC_H__9820EFEA_1038_4AB4_AB7E_997758F992C7__INCLUDED_)
#define AFX_RichODBC_H__9820EFEA_1038_4AB4_AB7E_997758F992C7__INCLUDED_

#include <windows.h>
#include <Sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>

#include <time.h>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_QUERY_SIZE	3072

typedef	VOID	( *pExternErrorHandler )( CHAR *pszQuery, LONG siParam1 );

class RichODBC  
{
public:
	RichODBC();
	~RichODBC();

	void		InitODBC();
	void		ReleaseODBC();

	VOID		SetParam(int siParamNo, int siType, void *pData, int siDatasize, SQLSMALLINT siParamType = SQL_PARAM_INPUT, bool bNull = false);
	int			GetData(int iColNo, int iType, void *pvData, long siDataSize);
	int			GetData(int iColNo, void *pvData, long siDataSize);
public:
	VOID		GetResult(VOID *pvResult, long siDatasize);
	const int	GetRowcount();
	const int	GetRetcode() { return m_retcode; }

	VOID		SetPooling(BOOL data) { IsPooling = data; }

private:
	BOOL		bReconnectStat;
	BOOL		bCanFetchRow;
	BOOL		bIsConnected;

public:
	void		inline SetReconnect(BOOL bStat) { bReconnectStat = bStat; }
	BOOL		inline GetReconnectStat() { return bReconnectStat; }

	BOOL		Connect(char *cDriver, char *cServer, char *cDatabase, char *cUserID, char *cPasswd);
	BOOL		ConnectProcess();
	BOOL		Reconnect();
	int			Disconnect();

	VOID		Prepare();
	int			ExecSQL();

	bool		FetchData();
	bool 		NextRecordSet();

	int			SetQuery(char *pszQuery);

	//////////////////////////////////////////////////////////
	void		BeginSP(char *pszQuery);
	void		EndSP();
	void		SetParam(int siType, void *pData, int siDatasize, SQLSMALLINT siParamType = SQL_PARAM_INPUT, bool bNull = false);
	int			GetData(void *pvData, long siDataSize = 0);
	/////////////////////////////////////////////////////////*/
	
	BOOL		CheckMDAC(char *cRequire);
	BOOL		CheckError(SQLSMALLINT hType, SQLHANDLE handle,char* hint=NULL, HWND hwnd = NULL);
	BOOL		IsSuccess(void);

	int			m_siParamNo;
	int			m_sicount;
	SQLINTEGER	m_siParamSize;

//protected:
	SQLHDBC		m_hdbc;
	SQLHSTMT	m_hstmt;
	SQLHENV		m_henv;

	SQLUINTEGER	m_siRowcount;
	SQLINTEGER	m_siNTS, m_siZero, m_siNULL;
	char		m_cConnectionString[1024];
	char		*m_pszQuery;

	time_t		m_tBEGINSP;	
	time_t		m_tEXECSP;
	time_t		m_tFETCH;
	time_t		m_tNEXTRECORDSET;

	pExternErrorHandler		m_pExternErrorHandler;
	VOID		SetExternErrorHandler( pExternErrorHandler pexternerrorhandler );


private:
	RETCODE		m_retcode;
	long		m_lbytes;
	BOOL		IsPooling;
	
private:
	FILE		*m_Queryfp;
	char		m_cErrQuery[8192];
	void		PrintErrMsg(char *msg);
	void		PrintQuery(char *msg);
	void		PrintParam(char *msg);
	void		WriteQuery();	
	void		WriteMSG(char* msg, size_t length);

public:
	void MakeTime( TIMESTAMP_STRUCT& src, time_t& dest );
	void MakeTIMESTAMP_STRUCT( time_t& src, TIMESTAMP_STRUCT& dest );

};


class XDBException
{
public:
	int code;
	char msg[4096];

	XDBException():code(0) { msg[0]='\0'; };

	XDBException(int c,const char * m):code(0) 
	{
		strncpy_s( msg, m, sizeof(msg) );
		//#ifndef TEST
		//	  Log & log=*Log::Instance();
		//	  log("XDBException:(%d:%s)\n",code,msg);
		//#endif
	};
};

#endif // !defined(AFX_RichODBC_H__9820EFEA_1038_4AB4_AB7E_997758F992C7__INCLUDED_)
