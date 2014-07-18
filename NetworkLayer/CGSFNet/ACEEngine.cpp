#include "ACEEngine.h"
#include "ACEHeader.h"
#include "ProactorWorkerThread.h"
#include "ProactorAcceptor.h"
#include "SingltonObject.h"
#include <assert.h>
#include "BasePacket.h"

#ifdef _DEBUG
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "ace.lib")
#endif

INetworkEngine * CreateNetworkEngine(bool bServer, IEngine* pEngine)
{
	if (bServer)
		return new ACEServerEngine(pEngine);
	else
		return new ACEClientEngine(pEngine);
}

ACEEngine::ACEEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
	, m_TimeOutHandler(this)
	, m_acceptorIndex(0)
{
}

ACEEngine::~ACEEngine(void)
{
}

bool ACEEngine::Init()
{
	ACE::init();

	return true;
}

bool ACEEngine::SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
{
	ProactorServiceManagerSinglton::instance()->SendInternal(ownerSerial, buffer, bufferSize);

	return true;
}

bool ACEEngine::Disconnect(int serial)
{
	ProactorServiceManagerSinglton::instance()->Disconnect(serial);
	return true;
}

bool ACEEngine::CheckTimerImpl()
{
	return true;
}

bool ACEEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
{
	ACE_Time_Value Interval(Period / 1000, (Period % 1000) * 1000);
	ACE_Time_Value Start(StartTime / 1000, (StartTime % 1000) * 1000);

	if (ACE_Proactor::instance()->schedule_timer(m_TimeOutHandler,
		(void*)TimerID,
		Start,
		Interval) == -1)
		return false;

	return true;
}

int ACEEngine::AddConnector(char* szIP, unsigned short port)
{
	ACE_SOCK_Stream* stream = new ACE_SOCK_Stream();
	ACE_INET_Addr connectAddr(port, szIP);
	ACE_SOCK_Connector connector;
	int result = connector.connect(*stream, connectAddr);
	if (-1 == result)
		return -1;

	ProactorService* pService = new ProactorService();
	pService->SetOwner(this);

	ACE_Message_Block mb;
	pService->open(stream->get_handle(), mb);
	delete stream;
	stream = NULL;

	m_mapConnector.insert(std::make_pair(pService->GetSerial(), pService));

	return pService->GetSerial();
}

int ACEEngine::AddListener(char* szIP, unsigned short port)
{
	ACE_INET_Addr listen_addr;
	listen_addr.set(port);

	ProactorAcceptor* pAcceptor = new ProactorAcceptor(this);

	if (0 != pAcceptor->open(listen_addr, 0, 1, ACE_DEFAULT_BACKLOG, 1, 0, 1, 1, 1024))
	{
		delete pAcceptor;
		return -1;
	}

	m_acceptorIndex++;

	pAcceptor->SetAcceptorNum(m_acceptorIndex);

	m_mapAcceptor.insert(std::make_pair(m_acceptorIndex, pAcceptor));

	return m_acceptorIndex;
}

bool ACEEngine::Shutdown()
{
	ACE_Proactor::instance()->close();
	ACE_Thread_Manager::instance()->wait_grp(m_workThreadGroupID);

	ACE::fini();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
ACEServerEngine::ACEServerEngine(IEngine* pEngine)
: ACEEngine(pEngine)
{

}

bool ACEServerEngine::Start(char* szIP, unsigned short port)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	int OptimalThreadCount =si.dwNumberOfProcessors * 2;

	m_workThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactorWorkerThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if (m_workThreadGroupID == -1)
	{
		return false;
	}

	return AddListener(szIP, port) > 0;
}

ACEClientEngine::ACEClientEngine(IEngine* pEngine)
: ACEEngine(pEngine)
{

}

bool ACEClientEngine::Start(char* szIP, unsigned short port)
{
	int OptimalThreadCount = 1;

	m_workThreadGroupID = ACE_Thread_Manager::instance()->spawn_n(OptimalThreadCount, (ACE_THR_FUNC)ProactorWorkerThread, NULL, THR_NEW_LWP, ACE_DEFAULT_THREAD_PRIORITY, 1);

	if (m_workThreadGroupID == -1)
	{
		assert(0);
		return FALSE;
	}

	return AddConnector(szIP, port) >= 0;
}