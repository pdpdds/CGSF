#pragma once
#include "SFDatabaseMySQL.h"

class SFDBProcessorMySQL : public SFDatabaseMySQL
{
public:
	SFDBProcessorMySQL(void);
	virtual ~SFDBProcessorMySQL(void);

	virtual BOOL Initialize() override;
	virtual BOOL RegisterDBService() override;

	virtual BasePacket* Alloc() override;
	virtual BOOL Release(BasePacket* pMessage) override;

	virtual BOOL Call(BasePacket* pMessage);

protected:

private:
	BOOL OnLogin( BasePacket* pMessage );
};


