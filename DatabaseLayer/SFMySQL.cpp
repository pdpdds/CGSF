#include "StdAfx.h"
#include "SFMySQL.h"
#include "StringConversion.h"
#include <functional>
#include "Macro.h"
#include "MySQLPool.h"

SFMySQL::~SFMySQL(void)
{
	if(m_pMysqlPool)
	{
        m_pMysqlPool->DestroyPool();      
	}

}

BOOL SFMySQL::Initialize(_DBConnectionInfo* pInfo)
{

	std::string szDBUrl = "tcp://" + StringConversion::ToASCII(pInfo->IP) + ":" + std::to_string(pInfo->Port);
	
	try{        

		 m_pMysqlPool = new MySQLPool(szDBUrl.c_str(), //database url                                                
			                          StringConversion::ToASCII(pInfo->szUser).c_str(),        //username                                                
									  StringConversion::ToASCII(pInfo->szPassword).c_str(),        //password                                               ]
									  300,            //keepalive timeout (seconds)                                                
									  "select 0 from dual"); //keepalive statement

 
        if(m_pMysqlPool->CreatePool(5, 7) <= 0)
		{
			LOG(FATAL) << "Mysql Create Pool Error";
			return FALSE;
		}

    }catch(sql::SQLException &e)
	 {
		 LOG(FATAL) << "Mysql Exception" << e.what();
         return FALSE;
    }
	
	return TRUE;
}

BOOL SFMySQL::Execute(char* szQuery)
{
	



	return TRUE;
}

