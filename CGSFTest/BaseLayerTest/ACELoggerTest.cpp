#include "stdafx.h"
#include "ACELoggerTest.h"

#include <ace/ace.h>
#include <ace/Task.h>
#include "ACELogger.h"

class MyCallback : public ACE_Log_Msg_Callback
{
public:
	void log(ACE_Log_Record& log_record)
	{
		ACE_Time_Value tv = log_record.time_stamp();
		FILETIME ft(tv);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);
		st.wHour += 9;
		printf("%02d-%02d %02d:%02d:%02d %s", 
			st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, log_record.msg_data());
	}
};

class LoggingTask : public ACE_Task_Base
{
public:
	int svc()
	{
		ACE_DEBUG((LM_INFO, "ProcessId(%P) ThreadId(%t) In Task\n"));
		return 0;
	}
};

ACELoggerTest::ACELoggerTest(void)
{
}


ACELoggerTest::~ACELoggerTest(void)
{
}

bool ACELoggerTest::Run()
{
	ACE::init();

	// Process/Thread Id 출력
	ACE_DEBUG((LM_INFO, "ProcessId(%P) ThreadId(%t)\n"));
	LoggingTask task;
	task.activate(THR_NEW_LWP, 5);

	Sleep(1000);

	// Source code 정보 출력
	ACE_DEBUG((LM_INFO, "%N %l Source code Info\n"));

	// 날짜/시간 정보 출력
	ACE_DEBUG((LM_INFO, "%D Date Info\n"));

	// 로깅 출력방식 변경
	// 파일로 변경
	ACE_LOGGER->RedirectToFile(L"FileLog.txt");
	ACE_DEBUG((LM_INFO, "%D Date Info To File\n"));

	// 역호출 객체(Callback)로 변경
	ACE_LOGGER->RedirectToCallback(new MyCallback);
	ACE_DEBUG((LM_INFO, "Date Info To Callback\n"));
	
	getchar();
	ACE::fini();

	return true;
}