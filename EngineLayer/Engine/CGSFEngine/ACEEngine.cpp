#include "ACEEngine.h"
#include "ACEHeader.h"
#include "ProactorWorkerThread.h"
#include "ProactorAcceptor.h"
#include "SingltonObject.h"
#include <assert.h>
#include "BasePacket.h"

INetworkEngine * CreateNetworkEngine(bool Server, IEngine* pEngine)
{
	if(Server)
		return new ACEServerEngine(pEngine);
	else
		return new ACEClientEngine(pEngine);
}

ACEServerEngine::ACEServerEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
	, m_Acceptor(this)
	, m_TimeOutHandler(this)
{
}


ACEServerEngine::~ACEServerEngine(void)
{
}

bool ACEServerEngine::Init()
{
	ACE::init();

	return true;
}

bool ACEServerEngine::Start(char* szIP, unsigned short Port)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	int OptimalThreadCount =si.dwNumberOfProcessors * 2;

	m_WorkThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactorWorkerThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(m_WorkThreadGroupID == -1)
	{
		return false;
	}

	//CreateTimerTask(pLogic);

	ACE_INET_Addr listen_addr;

	listen_addr.set(Port);

	if(0 != m_Acceptor.open(listen_addr, 0, 1, ACE_DEFAULT_BACKLOG, 1, 0, 1, 1, 1024))
		return false;

	return true;
}

bool ACEServerEngine::Shutdown()
{
	ACE_Thread_Manager::instance()->wait_grp(m_WorkThreadGroupID);

	//ACE_Thread_Manager::instance()->wait_grp(m_LogicThreadGroupID);
	ACE::fini();

	return true;
}

bool ACEServerEngine::SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
{
	 ProactorServiceMapSingleton::instance()->SendInternal(ownerSerial, buffer, bufferSize);

	return true;
}

bool ACEServerEngine::Disconnect(int Serial)
{	
	return true;
}

bool ACEServerEngine::CheckTimerImpl()
{
	return true;
}

bool ACEServerEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
{
	ACE_Time_Value Interval(Period/1000, (Period%1000)*1000);
	ACE_Time_Value Start(StartTime/1000, (StartTime%1000)*1000);

	if (ACE_Proactor::instance ()->schedule_timer (m_TimeOutHandler,
		(void*)TimerID,
		Start,
		Interval) == -1)
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

ACEClientEngine::ACEClientEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
	, m_TimeOutHandler(this)
{
}


ACEClientEngine::~ACEClientEngine(void)
{
}

bool ACEClientEngine::Init()
{
	ACE::init();

	int OptimalThreadCount = 1;

	m_WorkThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactorWorkerThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if(m_WorkThreadGroupID == -1)
	{
		assert(0);
		return FALSE;
	}

	return true;
}

bool ACEClientEngine::Start(char* szIP, unsigned short Port)
{
	ACE_SOCK_Stream* stream = new ACE_SOCK_Stream();
	ACE_INET_Addr connectAddr(Port, szIP);
	ACE_SOCK_Connector connector;
	int result = connector.connect(*stream, connectAddr);
	if (-1 == result)
		return false;

	m_ProactorService = new ProactorService();
	m_ProactorService->SetOwner(this);

	ACE_Message_Block mb;
	m_ProactorService->open(stream->get_handle(), mb);
	delete stream;
	stream = NULL;

	return true;
}

bool ACEClientEngine::Shutdown()
{
	ACE_Thread_Manager::instance()->wait_grp(m_WorkThreadGroupID);

	ACE::fini();

	return true;
}

bool ACEClientEngine::SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
{
	 ProactorServiceMapSingleton::instance()->SendInternal(ownerSerial, buffer, bufferSize);

	return true;
}

bool ACEClientEngine::Disconnect(int Serial)
{
	return true;
}

bool ACEClientEngine::CheckTimerImpl()
{
	return false;
}

bool ACEClientEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
{
	ACE_Time_Value Interval(Period/1000, (Period%1000)*1000);
	ACE_Time_Value Start(StartTime/1000, (StartTime%1000)*1000);

	if (ACE_Proactor::instance ()->schedule_timer (m_TimeOutHandler,
		(void*)TimerID,
		Start,
		Interval) == -1)
		return false;

	return true;
}