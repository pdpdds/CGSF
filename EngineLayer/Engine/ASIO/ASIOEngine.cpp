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
	m_pServer = new ChatServer(io_service, this);
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
	m_pServer->SendRequest(pPacket);
	return true;
}

bool ASIOServerEngine::Disconnect(int Serial)
{
	return true;
}

bool ASIOServerEngine::CheckTimerImpl()
{
	return false;
}

bool ASIOServerEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
{	
	return true;
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
	m_pClient = new ChatClient(io_service, this);

	return true;
}

bool ASIOClientEngine::Start(char* szIP, unsigned short Port)
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

bool ASIOClientEngine::SendRequest(BasePacket* pPacket)
{
	m_pClient->SendRequest(pPacket);
	return true;
}

bool ASIOClientEngine::Disconnect(int Serial)
{
	return true;
}

bool ASIOClientEngine::CheckTimerImpl()
{
	return false;
}

bool ASIOClientEngine::CreateTimerTask(unsigned int TimerID, unsigned int StartTime, unsigned int Period)
{	
	return true;
}