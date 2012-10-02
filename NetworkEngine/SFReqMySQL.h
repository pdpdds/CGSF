#pragma once

class SFReqMySQL
{
public:
	SFReqMySQL(void);
	virtual ~SFReqMySQL(void);

	BOOL Call(SFMessage* pMessage);
};
