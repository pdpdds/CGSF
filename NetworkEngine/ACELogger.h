#pragma once
#include <ace/Log_Msg.h>
#include <ace/Log_Record.h>
#include <ace/Log_Msg_Callback.h>
#include <ace/Singleton.h>

class ACELogger
{
private:
	std::ofstream* log_stream_;
	ACE_OSTREAM_TYPE* output_stream_;

public:
	ACELogger();
	virtual ~ACELogger();

	void ACELogger::Verbose(bool on);

	void RedirectToDaemon(const ACE_TCHAR* prog_name = ACE_TEXT(""));
	void RedirectToSyslog(const ACE_TCHAR* prog_name = ACE_TEXT(""));
	void RedirectToOStream(ACE_OSTREAM_TYPE* output);
	void RedirectToFile(const wchar_t* filename);
	void RedirectToStderr(void);
	ACE_Log_Msg_Callback* RedirectToCallback(ACE_Log_Msg_Callback* callback);
};

typedef ACE_Singleton<ACELogger, ACE_Null_Mutex> ACELoggerSingleton;
#define ACE_LOGGER ACELoggerSingleton::instance()
