#pragma once

class SFExceptionHandler
{
public:
	SFExceptionHandler(void){}
	virtual ~SFExceptionHandler(void){}

	virtual BOOL Install() = 0;

protected:

};
