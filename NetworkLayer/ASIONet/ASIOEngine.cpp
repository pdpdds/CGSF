#include "EngineInterface/EngineStructure.h"
#include "ASIOEngine.h"
#include "BasePacket.h"

INetworkEngine * CreateNetworkEngine(bool Server, IEngine* pEngine)
{
	if(Server)
		return new ASIOServerEngine(pEngine);
	else
		return new ASIOClientEngine(pEngine);
}

ASIOServerEngine::ASIOServerEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
	, m_pServer(NULL)
{
}

ASIOServerEngine::~ASIOServerEngine(void)
{
}

bool ASIOServerEngine::Init()
{
	m_pServer = new ASIOServer(io_service, this);
	m_pServer->Init( MAX_SESSION_COUNT );	
	return true;
}

bool ASIOServerEngine::Start(char* szIP, unsigned short Port)
{	
	m_pServer->Start();
	io_service.run();
	return true;
}

bool ASIOServerEngine::Shutdown()
{
	delete m_pServer;

	return true;
}

bool ASIOServerEngine::SendRequest(BasePacket* pPacket)
{
	return m_pServer->SendRequest(pPacket);
	
}

bool ASIOServerEngine::Disconnect(int serial)
{
	return true;
}

bool ASIOServerEngine::CheckTimerImpl()
{
	return false;
}

long ASIOServerEngine::AddTimer(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
{	
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////


ASIOClientEngine::ASIOClientEngine(IEngine* pEngine)
	: INetworkEngine(pEngine)
{
}

ASIOClientEngine::~ASIOClientEngine(void)
{
}

bool ASIOClientEngine::Init()
{	
	m_pClient = new ASIOClient(io_service, this);

	return true;
}

bool ASIOClientEngine::Start(char* szIP, unsigned short port)
{
	auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), PORT_NUMBER);
	m_pClient->Connect( endpoint );

	boost::thread thread( boost::bind(&boost::asio::io_service::run, &io_service) );
	return true;
}

bool ASIOClientEngine::Shutdown()
{
	delete m_pClient;
	return true;
}

bool ASIOClientEngine::Disconnect(int serial)
{
	return true;
}

bool ASIOClientEngine::CheckTimerImpl()
{
	return false;
}

long ASIOClientEngine::AddTimer(unsigned int timerID, unsigned int startTime, unsigned int period)
{	
	return -1;
}

bool ASIOClientEngine::SendRequest(BasePacket* pPacket)
{
	return m_pClient->SendRequest(pPacket);
}