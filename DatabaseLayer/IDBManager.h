#pragma once

class SFDBWorker;
class ACE_Method_Request;

class IDBManager
{
public:
	IDBManager(void){}
	virtual ~IDBManager(void){}

	virtual int return_to_work(SFDBWorker* pWorker, ACE_Method_Request* pReq) = 0;
	virtual bool done() = 0;
};
