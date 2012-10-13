#pragma once

class SFMessage;

class SFDatabase
{
public:
	SFDatabase(void);
	virtual ~SFDatabase(void);

	virtual BOOL Initialize(_DBConnectionInfo* pInfo) = 0;
	virtual BOOL Call(SFMessage* pMessage) = 0;
};