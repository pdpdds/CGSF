#include "EngineInterface/EngineStructure.h"
#include "ServerSession.h"
#include "ASIOServer.h"

Session::Session(int nSessionID, boost::asio::io_service& io_service, ASIOServer* pServer)
		: m_Socket(io_service)
		, m_nSessionID( nSessionID )
		, m_pServer( pServer )
{
	m_bCompletedWrite = true;
}

Session::~Session()
{
	while( m_SendDataQueue.empty() == false )
	{
		delete[] m_SendDataQueue.front();
		m_SendDataQueue.pop_front();
	}

	m_SendDataSizeQueue.clear();
}

void Session::Init()
{
	m_nPacketBufferMark = 0;
}

void Session::PostReceive()
{
	m_Socket.async_read_some
			( 
			boost::asio::buffer(m_ReceiveBuffer), 
			boost::bind( &Session::handle_receive, this, 
											boost::asio::placeholders::error, 
											boost::asio::placeholders::bytes_transferred ) 
				
		);
}

void Session::PostSend( const bool bImmediately, const int nSize, char* pData )
{
	char* pSendData = nullptr;

	if( bImmediately == false )
	{
		pSendData = new char[nSize];
		memcpy( pSendData, pData, nSize);

		m_SendDataQueue.push_back( pSendData );

		m_SendDataSizeQueue.push_back(nSize);
	}
	else
	{
		pSendData = pData;
	}

	

	if( m_bCompletedWrite == false )
	{
		return;
	}

	boost::asio::async_write( m_Socket, boost::asio::buffer( pSendData, nSize ),
							 boost::bind( &Session::handle_write, this,
								boost::asio::placeholders::error,
								boost::asio::placeholders::bytes_transferred )
							);
}

void Session::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
	delete[] m_SendDataQueue.front();
	m_SendDataQueue.pop_front();

	m_SendDataSizeQueue.pop_front();

	if( m_SendDataQueue.empty() == false )
	{
		m_bCompletedWrite = false;

		char* pData = m_SendDataQueue.front();
		
		int nSize = m_SendDataSizeQueue.front();

		PostSend(true, nSize, pData);
	}
	else
	{
		m_bCompletedWrite = true;
	}
}

void Session::handle_receive( const boost::system::error_code& error, size_t bytes_transferred )
{
	if( error )
	{
		if( error == boost::asio::error::eof )
		{
			std::cout << "클라이언트와 연결이 끊어졌습니다" << std::endl;
		}
		else 
		{
			std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
		}

		_SessionDesc desc;
		desc.sessionType = 0;
		desc.identifier = 1;

		m_pServer->CloseSession( m_nSessionID );
		OnDisconnect(m_nSessionID, desc);
	}
	else
	{
		_SessionDesc desc;
		desc.sessionType = 0;
		desc.identifier = 1;

		if (false == OnReceive(m_ReceiveBuffer.data(), bytes_transferred, desc))
		{
			//강제로 끊게 하는 메소드는?
		}

		PostReceive(); 
	}
}

bool Session::SendRequest(BasePacket* pPacket)
{
	IPacketProtocol* pProtocol = GetPacketProtocol();
	if (NULL == pProtocol)
		return false;

	char* pBuffer = NULL;
	int bufferSize;
	if (false == pProtocol->Encode(pPacket, &pBuffer, bufferSize))
		return false;

	PostSend(false, bufferSize, pBuffer);

	return true;
}
