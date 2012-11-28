#include "CGSFEngine.h"
#include "ACEHeader.h"
#include "ProactorWorkerThread.h"
#include "ProactorAcceptor.h"
#include "SingltonObject.h"
#include <assert.h>
#include "BasePacket.h"

INetworkEngine * CreateNetworkEngine(bool Server, IEngine* pEngine)
{
	if(Server)
		return new CGSFServerEngine(pEngine);
	else
		return new CGSFClientEngine(pEngine);
}

CGSFServerEngine::CGSFServerEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
	, m_Acceptor(this)
	, m_TimeOutHandler(this)
{
}


CGSFServerEngine::~CGSFServerEngine(void)
{
}

bool CGSFServerEngine::Init()
{
	ACE::init();

	return true;
}

bool CGSFServerEngine::Start(char* szIP, unsigned short Port)
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

bool CGSFServerEngine::Shutdown()
{
	ACE_Thread_Manager::instance()->wait_grp(m_WorkThreadGroupID);

	//ACE_Thread_Manager::instance()->wait_grp(m_LogicThreadGroupID);
	ACE::fini();

	return true;
}

bool CGSFServerEngine::SendRequest(BasePacket* pPacket)
{
	 ProactorServiceMapSingleton::instance()->SendRequest(pPacket);

	return true;
}

bool CGSFServerEngine::Disconnect(int Serial)
{
	return true;
}

bool CGSFServerEngine::CheckTimerImpl()
{
	return true;
}

bool CGSFServerEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
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

CGSFClientEngine::CGSFClientEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
	, m_TimeOutHandler(this)
{
}


CGSFClientEngine::~CGSFClientEngine(void)
{
}

bool CGSFClientEngine::Init()
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

bool CGSFClientEngine::Start(char* szIP, unsigned short Port)
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

bool CGSFClientEngine::Shutdown()
{
	ACE_Thread_Manager::instance()->wait_grp(m_WorkThreadGroupID);

	ACE::fini();

	return true;
}

bool CGSFClientEngine::SendRequest(BasePacket* pPacket)
{
	 ProactorServiceMapSingleton::instance()->SendRequest(pPacket);

	return true;
}

bool CGSFClientEngine::Disconnect(int Serial)
{
	return true;
}

bool CGSFClientEngine::CheckTimerImpl()
{
	return false;
}

bool CGSFClientEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
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