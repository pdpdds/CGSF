#pragma once
#include <sql.h>
#include <sqlext.h>
#include "SFList.h"

#define SH_SQL_SUCCESS(ret) (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)

#define SH_SQL_MAX_COL 64
#define SH_SQL_MAX_PARAM 64

#define SH_SQL_MAX_QUERY  4096

#define DEFINE_QUERY(query) \
	virtual void SetQuery() {::lstrcpy(m_szQuery, query); };

#define START_BIND_PARAM() \
	virtual BOOL BindParameters() {

#define START_BIND_COLUMN() \
	virtual BOOL BindColumns() {

#define BIND_INPUT_PARAM(param) \
	if (!SFStatement::BindParam(param, sizeof(param), SQL_PARAM_INPUT)) return FALSE;

#define BIND_INOUT_PARAM(param) \
	if (!SFStatement::BindParam(param, sizeof(param), SQL_PARAM_INPUT_OUTPUT)) return FALSE;

#define BIND_OUTPUT_PARAM(param) \
	if (!SFStatement::BindParam(param, sizeof(param), SQL_PARAM_OUTPUT)) return FALSE;

#define BIND_COLUMN(column) \
	if (!SFStatement::BindCol(column, sizeof(column))) return FALSE;

#define END_BIND() \
	return TRUE; };

#define ADD_STATEMENT(db, statement) \
	if(!db.AddStatement(statement)) \
	{ LogDBError(TRUE, statement); return FALSE; }



class SFStatement
{
	friend class SFMSSQL;
public:
	SFStatement(void);
	virtual ~SFStatement(void);

	BOOL GetDBError(LPSTR szMsg, int nBufLen);

	BOOL SetNull(SQLPOINTER pValue);

	BOOL IsNull(SQLPOINTER pValue);

protected:
	friend class SFQuery;

	BOOL BindParam(LPTSTR value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(LPSTR value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(TIMESTAMP_STRUCT& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(double& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(float& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(unsigned long& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(long& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(unsigned short& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(short& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(unsigned char& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(char& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(unsigned int& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(int& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);
	BOOL BindParam(__int64& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType);

	BOOL BindCol(LPTSTR value, SQLUINTEGER uLen);
	BOOL BindCol(LPSTR value, SQLUINTEGER uLen);
	BOOL BindCol(TIMESTAMP_STRUCT& value, SQLUINTEGER uLen);
	BOOL BindCol(double& value, SQLUINTEGER uLen);
	BOOL BindCol(float& value, SQLUINTEGER uLen);
	BOOL BindCol(unsigned long& value, SQLUINTEGER uLen);
	BOOL BindCol(long& value, SQLUINTEGER uLen);
	BOOL BindCol(unsigned short& value, SQLUINTEGER uLen);
	BOOL BindCol(short& value, SQLUINTEGER uLen);
	BOOL BindCol(unsigned char& value, SQLUINTEGER uLen);
	BOOL BindCol(char& value, SQLUINTEGER uLen);
	BOOL BindCol(unsigned int& value, SQLUINTEGER uLen);
	BOOL BindCol(int& value, SQLUINTEGER uLen);
	BOOL BindCol(__int64& value, SQLUINTEGER uLen);

	void DestroyStatement();
	BOOL CreateStatement(SQLHANDLE hStmt);
	void GetErrMsg(SQLCHAR* szMsg, SQLSMALLINT nBufLen);
	void ClearParameter();
	SQLHANDLE GetHandle();
	void TrimParameter();
	void TrimColumn();

	virtual BOOL BindParameters();
	virtual BOOL BindColumns();
	virtual void SetQuery();

	void _SetError(SQLRETURN sqlReturn);
	void _Trim(LPTSTR szString);

	BOOL _BindParam(SQLPOINTER pValue, SQLSMALLINT sInOutType, SQLSMALLINT sValueType, SQLSMALLINT sParamType, SQLUINTEGER uLen);
	BOOL _BindCol(SQLPOINTER pValue, SQLSMALLINT sValueType, SQLUINTEGER uLen);

	SQLHANDLE m_hStmt;
	SQLUSMALLINT m_usBindParam;
	SQLUSMALLINT m_usBindCol;
	SQLRETURN m_sqlReturn;
	SQLPOINTER m_arrayParamData[SH_SQL_MAX_PARAM];
	SQLINTEGER m_arrayParamInd[SH_SQL_MAX_PARAM];
	SQLSMALLINT m_arrayParamType[SH_SQL_MAX_PARAM];
	SQLPOINTER m_arrayColData[SH_SQL_MAX_COL];
	SQLINTEGER m_arrayColInd[SH_SQL_MAX_COL];
	SQLSMALLINT m_arrayColType[SH_SQL_MAX_COL];

	SQLWCHAR m_szQuery[SH_SQL_MAX_QUERY];

private:
};

class SFQuery
{
public:
	SFQuery(SFStatement& statement);
	virtual ~SFQuery();

	int RowCount();
	void Close();
	BOOL Fetch();
	BOOL Execute();

protected:

private:
	BOOL m_bIsLocked;
	BOOL m_bIsFirst;
	SFStatement* m_pStatement;
};


class SFMSSQL
{
public:
	SFMSSQL(void);
	virtual ~SFMSSQL(void);

	BOOL Initialize(_DBConnectionInfo* pInfo);
	/////////////////////////////////////////////////////////
//Method
/////////////////////////////////////////////////////////
	BOOL ConnectDB(char* pDBName, char* pID, char* pPassword);
	BOOL RegisterStatement(SFStatement& statement);
	BOOL IsDBConnected();

	BOOL GetDBError(LPSTR szMsg, int nBufLen);
	void Disconnect();
	SQLHANDLE Connect(char* szDSN, char* szUserID, char* szPasswd, SQLHANDLE hEnv = SQL_NULL_HANDLE);
	BOOL AddStatement(SFStatement& statement);
	BOOL IsConnected(){return m_bIsConnected;}

protected:

private:
	SQLHANDLE m_hDbc;
	SQLHANDLE m_hEnv;
	SQLRETURN m_sqlReturn;
	BOOL m_bIsConnected;
	SFList m_listStatement;
};
