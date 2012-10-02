#include "StdAfx.h"
#include "GoogleLog.h"

GoogleLog::GoogleLog(void)
{
}

GoogleLog::~GoogleLog(void)
{
}

BOOL GoogleLog::Initialize()
{
	//google::InitGoogleLogging("CGSF");

	//google::SetLogDestination(GLOG_ERROR, "./");

	return TRUE;
}

BOOL GoogleLog::Finally()
{
	//google::ShutdownGoogleLogging();

	return TRUE;
}