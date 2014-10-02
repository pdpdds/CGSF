#pragma once

#define DEFAUT_POOL_SIZE 5

typedef struct tag_DBModuleParams
{
	bool threadSafe; 
	int workerPoolSize;

	tag_DBModuleParams()
	{
		threadSafe = true;
		workerPoolSize = DEFAUT_POOL_SIZE;
	}

}DBModuleParams;

class BasePacket;

class SFDatabaseProxy
{
public:
	SFDatabaseProxy(void){}
	virtual ~SFDatabaseProxy(void){}

	virtual BOOL Initialize(DBModuleParams& param) = 0;
	virtual BOOL SendDBRequest(BasePacket* pMessage) = 0;

protected:

private:
};
