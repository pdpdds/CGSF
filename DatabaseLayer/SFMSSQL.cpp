#include "StdAfx.h"
#include "SFMSSQL.h"
#include "StringConversion.h"

SFMSSQL::SFMSSQL(void)
{
	m_hEnv = SQL_NULL_HANDLE;
	m_hDbc = SQL_NULL_HANDLE;
	m_sqlReturn = 0;
	m_bIsConnected = FALSE;
}

SFMSSQL::~SFMSSQL(void)
{
	if(IsDBConnected() == TRUE)
		Disconnect();

	if(m_hDbc != SQL_NULL_HANDLE)
	{
		if(m_bIsConnected)
			::SQLDisconnect(m_hDbc);
		::SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
	}
	if(m_hEnv != SQL_NULL_HANDLE)
		::SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
}

BOOL SFMSSQL::Initialize(_DBConnectionInfo* pInfo)
{
	if(FALSE == ConnectDB((char*)StringConversion::ToASCII(pInfo->szDBName).c_str(), 
		                  (char*)StringConversion::ToASCII(pInfo->szUser).c_str(), 
						  (char*)StringConversion::ToASCII(pInfo->szPassword).c_str()))
		return FALSE;

	return TRUE;
}

BOOL SFMSSQL::ConnectDB(char* pDBName, char* pID, char* pPassword)
{
	Disconnect();

	if(SQL_NULL_HANDLE == Connect(pDBName, pID, pPassword))
		return FALSE;

	return TRUE;
}

BOOL SFMSSQL::IsDBConnected()
{
	return IsConnected();
}	

BOOL SFMSSQL::RegisterStatement(SFStatement& statement)
{
	if(!AddStatement(statement))
		return FALSE;

	return TRUE;
}

SQLHANDLE SFMSSQL::Connect(char* szDSN, char* szUserID, char* szPassword, SQLHANDLE hEnv)
{
	//if(SQL_NULL_HANDLE)
	{
		m_sqlReturn = ::SQLSetEnvAttr(NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);
		if(!SH_SQL_SUCCESS(m_sqlReturn))
			return SQL_NULL_HANDLE;

		m_sqlReturn = ::SQLAllocHandle(SQL_HANDLE_ENV, NULL, &hEnv);
		if(!SH_SQL_SUCCESS(m_sqlReturn))
			return SQL_NULL_HANDLE;

		m_sqlReturn = ::SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
		if(!SH_SQL_SUCCESS(m_sqlReturn))
			return SQL_NULL_HANDLE;

		m_sqlReturn = ::SQLSetEnvAttr(hEnv, SQL_ATTR_CP_MATCH, (SQLPOINTER)SQL_CP_RELAXED_MATCH, SQL_IS_INTEGER);
		if(!SH_SQL_SUCCESS(m_sqlReturn))
			return SQL_NULL_HANDLE;

		m_hEnv = hEnv;
	}

	m_sqlReturn = ::SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &m_hDbc);
	if(!SH_SQL_SUCCESS(m_sqlReturn))			
		return SQL_NULL_HANDLE;

	m_sqlReturn = ::SQLConnectA(m_hDbc, (SQLCHAR*)szDSN, SQL_NTS, (SQLCHAR*)szUserID, SQL_NTS, (SQLCHAR*)szPassword, SQL_NTS);
	if(SH_SQL_SUCCESS(m_sqlReturn))
		m_bIsConnected = TRUE;
	else
	{
		hEnv = SQL_NULL_HANDLE;

		if(m_hEnv != SQL_NULL_HANDLE)
		{
			::SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
			m_hEnv = SQL_NULL_HANDLE;
		}
	}

	return m_hEnv;
}

BOOL SFMSSQL::AddStatement(SFStatement& statement)
{
	if(m_hDbc == SQL_NULL_HANDLE)
		return FALSE;

	SQLHANDLE hStmt;
	m_sqlReturn = ::SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
	if(!SH_SQL_SUCCESS(m_sqlReturn))
		return FALSE;

	if(!statement.CreateStatement(hStmt))
		return FALSE;

	m_listStatement.AddTail(&statement);

	return TRUE;
}

void SFMSSQL::Disconnect()
{
	while (!m_listStatement.IsEmpty())
	{
		SFStatement* pStatement = (SFStatement*)m_listStatement.RemoveHead();
		pStatement->DestroyStatement();
	}

	if(m_hDbc != SQL_NULL_HDBC)
	{
		if (m_bIsConnected)
			::SQLDisconnect(m_hDbc);
		::SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
	}

	m_bIsConnected = FALSE;
	m_hDbc = SQL_NULL_HANDLE;
}

BOOL SFMSSQL::GetDBError(LPSTR szMsg, int nBufLen)
{
	if(m_sqlReturn == SQL_INVALID_HANDLE)
	{
		::lstrcpyA(szMsg, "Invalid Handle");
		return TRUE;
	}
	else if(m_sqlReturn == SQL_ERROR)
	{
		SQLCHAR szState[5];
		SQLINTEGER nNativeError;
		SQLSMALLINT sLen;
		return SH_SQL_SUCCESS(::SQLGetDiagRecA(SQL_HANDLE_DBC, m_hDbc, 1, szState, &nNativeError, (SQLCHAR*)szMsg, nBufLen, &sLen));
	}

	return FALSE;
}

SFStatement::SFStatement()
{
	m_hStmt = SQL_NULL_HANDLE;
	m_sqlReturn = 0;
	m_usBindParam = 0;
	m_usBindCol = 0;

	::memset(m_arrayParamData, NULL, sizeof(SQLPOINTER) * SH_SQL_MAX_PARAM);
	::memset(m_arrayParamInd, NULL, sizeof(SQLINTEGER) * SH_SQL_MAX_PARAM);
	::memset(m_arrayColData, NULL, sizeof(SQLPOINTER) * SH_SQL_MAX_COL);
	::memset(m_arrayColInd, NULL, sizeof(SQLINTEGER) * SH_SQL_MAX_COL);

	m_szQuery[0] = '\0';
}

SFStatement::~SFStatement()
{
	if(m_hStmt != SQL_NULL_HANDLE)
		::SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
}

BOOL SFStatement::CreateStatement(SQLHANDLE hStmt)
{
	if(m_hStmt != SQL_NULL_HANDLE)
		return FALSE;

	m_hStmt = hStmt;
	m_usBindParam = 0;
	m_usBindCol = 0;

	SetQuery();

	m_sqlReturn = ::SQLPrepare(m_hStmt, m_szQuery, SQL_NTS);
	if(!SH_SQL_SUCCESS(m_sqlReturn))
		return FALSE;

	return (BindParameters() && BindColumns());
}

void SFStatement::DestroyStatement()
{
	if(m_hStmt != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
		m_hStmt = SQL_NULL_HANDLE;
	}
}

BOOL SFStatement::BindParameters()
{
	return TRUE;
}

BOOL SFStatement::BindColumns()
{
	return TRUE;
}

void SFStatement::SetQuery()
{
}

BOOL SFStatement::_BindParam (SQLPOINTER pValue, 
							  SQLSMALLINT sInOutType, 
							  SQLSMALLINT sValueType,
							  SQLSMALLINT sParamType,
							  SQLUINTEGER uLen)
{
	if(m_usBindParam >= SH_SQL_MAX_PARAM)
		return FALSE;

	m_arrayParamData[m_usBindParam] = pValue;
	m_arrayParamType[m_usBindParam] = sValueType;
	SQLUINTEGER uColSize = 0;

	switch (sParamType)
	{
	case SQL_CHAR:
		uColSize = uLen - 1;
		break;

	case SQL_TYPE_TIMESTAMP:
		uColSize = 19;
		break;

	case SQL_FLOAT:
		uColSize = 15;
		break;

	case SQL_REAL:
		uColSize = 7;
		break;
	}

	if(sParamType == SQL_CHAR && SQL_PARAM_INPUT == sInOutType)
	{
		m_sqlReturn = ::SQLBindParameter (m_hStmt, 
			                              m_usBindParam + 1, 
										  sInOutType, 
										  sValueType, 
										  sParamType, 
										  uColSize, 
										  0, 
										  m_arrayParamData[m_usBindParam], 
										  uLen, 
										  &(m_arrayParamInd[m_usBindParam]));
		m_usBindParam++;
	}

	return SH_SQL_SUCCESS(m_sqlReturn);
}



BOOL SFStatement::BindParam(int& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_SLONG, SQL_INTEGER, 0);
}

BOOL SFStatement::BindParam(unsigned int& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_ULONG, SQL_INTEGER, 0);
}

BOOL SFStatement::BindParam(char& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_STINYINT, SQL_TINYINT, 0);
}

BOOL SFStatement::BindParam(unsigned char& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_UTINYINT, SQL_TINYINT, 0);
}

BOOL SFStatement::BindParam(short& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_SSHORT, SQL_SMALLINT, 0);
}

BOOL SFStatement::BindParam(unsigned short& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_USHORT, SQL_SMALLINT, 0);
}

BOOL SFStatement::BindParam(long& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_SLONG, SQL_INTEGER, 0);
}

BOOL SFStatement::BindParam(unsigned long& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_ULONG, SQL_INTEGER, 0);
}

BOOL SFStatement::BindParam(float& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_FLOAT, SQL_REAL, 0);
}

BOOL SFStatement::BindParam(double& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_DOUBLE, SQL_FLOAT, 0);
}

BOOL SFStatement::BindParam(__int64& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_SBIGINT, SQL_BIGINT, 0);
}

BOOL SFStatement::BindParam(TIMESTAMP_STRUCT& value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0);
}


BOOL SFStatement::BindParam(LPTSTR value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_TCHAR, SQL_CHAR, 0);
}

BOOL SFStatement::BindParam(LPSTR value, SQLUINTEGER uLen, SQLSMALLINT sInOutType)
{
	return _BindParam(&value, sInOutType, SQL_C_CHAR, SQL_CHAR, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////
//Bin Columns
////////////////////////////////////////////////////////////////////////////////////////////

BOOL SFStatement::_BindCol(SQLPOINTER pValue, SQLSMALLINT sValueType, SQLUINTEGER uLen)
{
	if(m_usBindParam >= SH_SQL_MAX_PARAM)
		return FALSE;

	m_arrayColData[m_usBindCol] = pValue;
	m_arrayColType[m_usBindCol] = sValueType;
	

	m_sqlReturn = ::SQLBindCol (m_hStmt, 
		                        m_usBindCol + 1,
								sValueType,
								m_arrayColData[m_usBindCol], uLen, &(m_arrayColInd[m_usBindCol]));
	m_usBindCol++;

	return SH_SQL_SUCCESS(m_sqlReturn);							  
}

BOOL SFStatement::BindCol(int& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_SLONG, 0);
}

BOOL SFStatement::BindCol(unsigned int& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_ULONG, 0);
}

BOOL SFStatement::BindCol(char& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_STINYINT, 0);
}

BOOL SFStatement::BindCol(unsigned char& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_UTINYINT, 0);
}

BOOL SFStatement::BindCol(short& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_SSHORT, 0);
}

BOOL SFStatement::BindCol(unsigned short& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_USHORT, 0);
}

BOOL SFStatement::BindCol(long& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_SLONG, 0);
}

BOOL SFStatement::BindCol(unsigned long& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_ULONG, 0);
}

BOOL SFStatement::BindCol(float& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_FLOAT, 0);
}

BOOL SFStatement::BindCol(double& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_DOUBLE, 0);
}

BOOL SFStatement::BindCol(__int64& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_SBIGINT, 0);
}

BOOL SFStatement::BindCol(TIMESTAMP_STRUCT& value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_TYPE_TIMESTAMP, 0);
}

BOOL SFStatement::BindCol(LPSTR value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_CHAR, uLen);
}

BOOL SFStatement::BindCol(LPTSTR value, SQLUINTEGER uLen)
{
	return _BindCol(&value, SQL_C_TCHAR, uLen);
}

BOOL SFStatement::SetNull(SQLPOINTER pValue)
{
	for (SQLUSMALLINT i = 0; i < m_usBindParam; i++)
	{
		if (m_arrayParamData[i] == pValue)
		{
			m_arrayParamInd[i] = SQL_NULL_DATA;
			return TRUE;
		}
	}

	return FALSE;	
}

BOOL SFStatement::IsNull(SQLPOINTER pValue)
{
	for (SQLUSMALLINT i = 0; i < m_usBindParam; i++)
	{
		if (m_arrayParamData[i] == pValue)
		{
			return (m_arrayParamInd[i] = SQL_NULL_DATA);
			
		}
	}

	for (SQLUSMALLINT i = 0; i < m_usBindCol; i++)
	{
		if (m_arrayColData[i] == pValue)
		{
			return (m_arrayColInd[i] = SQL_NULL_DATA);
			
		}
	}

	return FALSE;	
}

SQLHANDLE SFStatement::GetHandle()
{
	return m_hStmt;
}

void SFStatement::_SetError(SQLRETURN sqlReturn)
{
	m_sqlReturn = sqlReturn;
}

BOOL SFStatement::GetDBError(LPSTR szMsg, int nBufLen)
{
	if (m_sqlReturn == SQL_INVALID_HANDLE)
	{
		::lstrcpyA(szMsg, "Invalid Handle");
	}
	else if(m_sqlReturn == SQL_ERROR)
	{
		SQLCHAR szState[5];
		SQLINTEGER nNativeError;
		SQLSMALLINT sLen;
		return SH_SQL_SUCCESS(::SQLGetDiagRecA(SQL_HANDLE_STMT, m_hStmt, 1, szState, &nNativeError, (SQLCHAR*)szMsg, nBufLen, &sLen));
	}

	return FALSE;
}

void SFStatement::ClearParameter()
{
	for(SQLUSMALLINT i = 0; i < m_usBindParam; i++)
	{
		if(m_arrayParamType[i] == SQL_C_CHAR)
			m_arrayParamInd[i] = SQL_NTS;
		else
			m_arrayParamInd[i] = 0;
	}
}

void SFStatement::TrimParameter()
{
	for (SQLUSMALLINT i = 0; i < m_usBindParam; i++)
	{
		if (m_arrayParamType[i] == SQL_C_CHAR && m_arrayParamInd[i] != SQL_NULL_DATA)
		{
			_Trim((LPTSTR)m_arrayParamData[i]);
		}

		if (m_arrayParamType[i] == SQL_C_TCHAR && m_arrayParamInd[i] != SQL_NULL_DATA)
		{
			_Trim((LPTSTR)m_arrayParamData[i]);
		}
	}	
}

void SFStatement::TrimColumn()
{
	for (SQLUSMALLINT i = 0; i < m_usBindCol; i++)
	{
		if (m_arrayColType[i] == SQL_C_CHAR && m_arrayColInd[i] != SQL_NULL_DATA)
		{
				_Trim((LPTSTR)m_arrayColData[i]);
		}
	}
}

void SFStatement::_Trim(LPTSTR szString)
{
	if(szString == NULL)
		return;

	int nLen = ::lstrlen(szString);

	for(LPTSTR szTail = szString + nLen - 1; szTail >= szString; szTail--)
	{
		if(*szTail == ' ')
			*szTail = '\0';
		else
			return;
	}
}

void SFStatement::GetErrMsg(SQLCHAR* szMsg, SQLSMALLINT nBufLen)
{
	if(m_sqlReturn == SQL_ERROR)
	{
		SQLCHAR szState[5];
		SQLINTEGER nNativeError;
		SQLSMALLINT nLen;
		::SQLGetDiagRecA(SQL_HANDLE_STMT, m_hStmt, 1, szState, &nNativeError, szMsg, nBufLen, &nLen);
	}
}

SFQuery::SFQuery(SFStatement& statement)
{
	m_pStatement = &statement;
	//m_bIsLocked = m_pStatement->Lock(TRUE);
	m_bIsFirst = TRUE;
	statement.ClearParameter();
}

SFQuery::~SFQuery()
{
	if(m_pStatement && m_bIsLocked)
	{
		::SQLFreeStmt(m_pStatement->GetHandle(), SQL_CLOSE);
		//m_pStatement->Unlock();
	}
}

BOOL SFQuery::Execute()
{
	if(m_pStatement == NULL || !m_bIsLocked)
		return FALSE;

	SQLRETURN sqlReturn = ::SQLExecute(m_pStatement->GetHandle());
	m_pStatement->_SetError(sqlReturn);

	if(!SH_SQL_SUCCESS(sqlReturn))
		return FALSE;

	while ((sqlReturn = ::SQLMoreResults(m_pStatement->GetHandle())) != SQL_NO_DATA)
	{
		if(!SH_SQL_SUCCESS(sqlReturn))
		{
			m_pStatement->_SetError(sqlReturn);
			return FALSE;
		}
	}

	m_pStatement->TrimParameter();
	return TRUE;
}

BOOL SFQuery::Fetch()
{
	if(m_pStatement == NULL || !m_bIsLocked)
		return FALSE;

	SQLRETURN sqlReturn;

	if(m_bIsFirst)
	{
		m_bIsFirst = FALSE;
		sqlReturn = ::SQLExecute(m_pStatement->GetHandle());
		m_pStatement->_SetError(sqlReturn);

		if(!SH_SQL_SUCCESS(sqlReturn))
		return FALSE;
	}

	sqlReturn = ::SQLFetch(m_pStatement->GetHandle());
	m_pStatement->_SetError(sqlReturn);

	if(!SH_SQL_SUCCESS(sqlReturn))
	{
		//SQLRETURN retTemp;
		//retTemp = ::SQLCloseCursor(m_pStatement->GetHandle());
		return FALSE;
	}

	m_pStatement->TrimColumn();
	return TRUE;
}

int SFQuery::RowCount()
{
	if(m_pStatement == NULL || !m_bIsLocked)
		return FALSE;

	SQLINTEGER nCount;
	SQLRETURN sqlReturn = ::SQLRowCount(m_pStatement->GetHandle(), &nCount);

	m_pStatement->_SetError(sqlReturn);

	if(!SH_SQL_SUCCESS(sqlReturn))
	{
		return -1;
	}

	return nCount;
}