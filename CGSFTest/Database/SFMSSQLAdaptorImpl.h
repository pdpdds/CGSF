#pragma once
#include "SFMSSQLAdaptor.h"

#define MAX_USER_NAME 20

class SFStmt_SPLoadUser : public SFStatement
{
public:
	int nErrorCode;
	int nUserID;
	char szUserName[MAX_USER_NAME];

	void Init(){ nErrorCode = nUserID = 0, memset(szUserName, 0, sizeof(char)* MAX_USER_NAME); }

protected:
	DEFINE_QUERY(L"{call SPLoadUser(?)}")

	START_BIND_PARAM()
		BIND_OUTPUT_PARAM(nErrorCode)
	END_BIND()

	START_BIND_COLUMN()
		BIND_COLUMN(nUserID)
		BIND_COLUMN(szUserName)
	END_BIND()
};


class SFMSSQLAdaptorImpl : public SFMSSQLAdaptor
{
public:
	SFMSSQLAdaptorImpl(void);
	virtual ~SFMSSQLAdaptorImpl(void);

	virtual BOOL RegisterDBService() override;

	BOOL OnLoadUser( BasePacket* pMessage );
	virtual bool RecallDBMsg(BasePacket* pMessage) override;
		
protected:		

private:
	BOOL AddStatement();

	SFStmt_SPLoadUser m_Stmt_SPLoadUser;
};
