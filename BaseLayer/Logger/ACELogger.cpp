#include "StdAfx.h"
#include "ACELogger.h"
#include "ACELogger.h"
#include "ace/Log_Msg_Callback.h"
#include <fstream>

ACELogger::ACELogger() : log_stream_(0), output_stream_(0)
{
}

ACELogger::~ACELogger()
{
	if (log_stream_)
		log_stream_->close();

	delete log_stream_;
}

void ACELogger::Verbose(bool on)
{
	if (on)
		ACE_LOG_MSG->set_flags(ACE_Log_Msg::VERBOSE_LITE);
	else 
		ACE_LOG_MSG->clr_flags(ACE_Log_Msg::VERBOSE_LITE);

}

void ACELogger::RedirectToDaemon(const ACE_TCHAR* prog_name)
{
	ACE_LOG_MSG->open(prog_name, ACE_Log_Msg::LOGGER, ACE_DEFAULT_LOGGER_KEY);
}

void ACELogger::RedirectToSyslog(const ACE_TCHAR* prog_name)
{
	ACE_LOG_MSG->open(prog_name, ACE_Log_Msg::SYSLOG, prog_name);
}

void ACELogger::RedirectToOStream(ACE_OSTREAM_TYPE* output)
{
	output_stream_ = output;
	ACE_LOG_MSG->msg_ostream (this->output_stream_);
	ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR | ACE_Log_Msg::LOGGER);
	ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
}

void ACELogger::RedirectToFile(const wchar_t* filename)
{
	log_stream_ = new ofstream ();
	log_stream_->open (filename, ios::out | ios::app);
	this->RedirectToOStream((ACE_OSTREAM_TYPE *)log_stream_);
}

void ACELogger::RedirectToStderr(void)
{
	ACE_LOG_MSG->clr_flags(ACE_Log_Msg::OSTREAM | ACE_Log_Msg::LOGGER);
	ACE_LOG_MSG->set_flags (ACE_Log_Msg::STDERR);
}

ACE_Log_Msg_Callback* ACELogger::RedirectToCallback(ACE_Log_Msg_Callback* callback)
{
	ACE_Log_Msg_Callback *previous = ACE_LOG_MSG->msg_callback (callback);
	if (callback == 0)
		ACE_LOG_MSG->clr_flags (ACE_Log_Msg::MSG_CALLBACK);
	else
		ACE_LOG_MSG->set_flags (ACE_Log_Msg::MSG_CALLBACK);

	return previous;
}