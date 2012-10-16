#include "ProactorWorkerThread.h"
#include "ACEHeader.h"

void ProactorWorkerThread(void* Args)
{
	ACE_Proactor::instance()->proactor_run_event_loop();
}