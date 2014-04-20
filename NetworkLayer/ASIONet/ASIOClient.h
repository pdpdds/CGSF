#include <SDKDDKVer.h>
#include <deque>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "Protocol.h"
#include <EngineInterface/INetworkEngine.h>
#include <EngineInterface/ISession.h>

#define ASIO_CLIENT_SESSION_ID 1

class ASIOClient : public ISession
{
public:
	ASIOClient(boost::asio::io_service& io_service, INetworkEngine* pEngine)
    : 	
	m_IOService(io_service),		  
	m_Socket(io_service),	  
	m_pOwner(pEngine),
	m_sessionId(ASIO_CLIENT_SESSION_ID)
	{
		InitializeCriticalSectionAndSpinCount(&m_lock, 4000);		
		SetOwner(pEngine);
	}

	~ASIOClient()
	{
		EnterCriticalSection(&m_lock);

		while( m_SendDataQueue.empty() == false )
		{
			delete[] m_SendDataQueue.front();
			m_SendDataQueue.pop_front();
		}

		m_SendDataSizeQueue.clear();

		LeaveCriticalSection(&m_lock);

		DeleteCriticalSection(&m_lock); 
	}
	
	bool IsConnecting() { return m_Socket.is_open(); }

	void Connect( boost::asio::ip::tcp::endpoint endpoint )
	{
		m_nPacketBufferMark = 0;

		m_Socket.async_connect( endpoint,
			boost::bind(&ASIOClient::handle_connect, this,
									boost::asio::placeholders::error)
					);
	}

	void Close()
	{
		if( m_Socket.is_open() )
		{
			m_Socket.close();
		}

		OnDisconnect(m_sessionId);
	}


	virtual void SendInternal(char* buffer, int bufferSize, int ownerSerial = -1)
	{
		PostSend(false, bufferSize, buffer);
	}

	void PostSend( const bool bImmediately, const int nSize, char* pData )
	{
		char* pSendData = nullptr;

		EnterCriticalSection(&m_lock);		// 락 시작
		
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

		if( bImmediately || m_SendDataQueue.size() < 2 )
		{
			boost::asio::async_write( m_Socket, boost::asio::buffer( pSendData, nSize ),
				boost::bind(&ASIOClient::handle_write, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::bytes_transferred )
									);
		}

		LeaveCriticalSection(&m_lock);		// 락 완료
	}

	

private:

	void PostReceive()
	{
		memset( &m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer) );

		m_Socket.async_read_some
					( 
						boost::asio::buffer(m_ReceiveBuffer), 
						boost::bind(&ASIOClient::handle_receive, this,
													boost::asio::placeholders::error, 
													boost::asio::placeholders::bytes_transferred ) 
				
					);
	}

	void handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{	
			std::cout << "서버 접속 성공" << std::endl;

			OnConnect(m_sessionId);
			PostReceive();
		}
		else
		{
			std::cout << "서버 접속 실패. error No: " << error.value() << " error Message: " << error.message() << std::endl;
		}
	}

	void handle_write(const boost::system::error_code& error, size_t bytes_transferred)
	{
		EnterCriticalSection(&m_lock);			// 락 시작

		delete[] m_SendDataQueue.front();
		m_SendDataQueue.pop_front();

		m_SendDataSizeQueue.pop_front();

		char* pData = nullptr;
		int nSize = 0;

		if( m_SendDataQueue.empty() == false )
		{
			pData = m_SendDataQueue.front();
			nSize = m_SendDataSizeQueue.front();
		}
		
		LeaveCriticalSection(&m_lock);			// 락 완료

		
		if (pData != nullptr && nSize != 0)
		{
			PostSend(true, nSize, pData);
		}
	}

	void handle_receive( const boost::system::error_code& error, size_t bytes_transferred )
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

			Close();
		}
		else
		{
			if(false == OnReceive(m_ReceiveBuffer.data(), bytes_transferred))
			{
			}			

			PostReceive(); 
		}
	}  

private:
	boost::asio::io_service& m_IOService;
	boost::asio::ip::tcp::socket m_Socket;

	std::array<char, 512> m_ReceiveBuffer;
	
	int m_nPacketBufferMark;
	char m_PacketBuffer[MAX_RECEIVE_BUFFER_LEN*2];

	CRITICAL_SECTION m_lock;
	std::deque< char* > m_SendDataQueue;
	std::deque< int > m_SendDataSizeQueue;

	INetworkEngine* m_pOwner;

	int m_sessionId;
};