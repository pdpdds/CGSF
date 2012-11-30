#pragma once

class SFMessage;

class SFReqMySQL
{
public:
	SFReqMySQL(void);
	virtual ~SFReqMySQL(void);

	BOOL Call(SFMessage* pMessage);
};
