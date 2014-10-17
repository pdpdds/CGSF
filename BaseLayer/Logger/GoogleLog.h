#pragma once

class GoogleLog
{
public:
	GoogleLog(void);
	virtual ~GoogleLog(void);

	bool Initialize()	{ return true; }
	bool Finally()		{ return true; }
};
