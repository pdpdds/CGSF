#pragma once

class SFMessage;

class SFDatabaseProxy
{
public:
	SFDatabaseProxy(void);
	virtual ~SFDatabaseProxy(void);

	virtual BOOL Initialize() = 0;
	virtual BOOL SendDBRequest(SFMessage* pMessage) = 0;

protected:

private:
};
