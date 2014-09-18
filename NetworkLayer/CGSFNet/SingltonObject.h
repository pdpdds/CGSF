#pragma once
#include "ACEHeader.h"
#include "ProactorService.h"
#include "ProactorServiceManager.h"

#define MAX_CGSF_CONCURRENT_USER 10000

typedef ACE_Singleton<ProactorServiceManager<ACE_Null_Mutex, ProactorService, MAX_CGSF_CONCURRENT_USER>, ACE_Thread_Mutex> ProactorServiceManagerSinglton;
