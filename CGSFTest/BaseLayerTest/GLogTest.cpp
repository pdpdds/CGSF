#include "stdafx.h"
#include "GLogTest.h"
#include <string>
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"

GLogTest::GLogTest(void)
{
}


GLogTest::~GLogTest(void)
{
}

bool GLogTest::Run()
{

	google::InitGoogleLogging("CGSF");

	google::SetLogDestination(GLOG_ERROR, "./");

	LOG(ERROR) << "This should work";
	LOG_IF(ERROR, 6 > 5) << "This should be also OK";

	LOG(INFO) << "info";
	LOG(WARNING) << "warn";
	LOG(ERROR) << "error CGSF";
	//LOG(FATAL) << "fatal";

	std::vector<std::string> dir = google::GetLoggingDirectories();
	std::vector<std::string>::iterator it;
	for(it=dir.begin(); it!=dir.end(); ++it){
		LOG(INFO) << *it;
	}	

	//DLOG(FATAL) << "CGSF";

	return true;
}