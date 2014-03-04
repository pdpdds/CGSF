#pragma once 

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>
#include <vector>


#include "ServerSession.h"
#include "Protocol.h"
#include "BasePacket.h"


class ChatServer
{
public:
	ChatServer( boost::asio::io_service& io_service, INetworkEngine* pEngine )
		: m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
		, m_pOwner(pEngine)
	{
		m_bIsAccepting = false;
	}

	~ChatServer()
	{
		for( size_t i = 0; i < m_SessionList.size(); ++i )
		{
			if( m_SessionList[i]->Socket().is_open() )
			{
				m_SessionList[i]->Socket().close();
			}

			delete m_SessionList[i];
		}
	}

	void Init( const int nMaxSessionCount )
	{
		for( int i = 0; i < nMaxSessionCount; ++i )
		{
			Session* pSession = new Session( i, m_acceptor.get_io_service(), this );
			pSession->SetOwner(m_pOwner);

			m_SessionList.push_back( pSession );
			m_SessionQueue.push_back( i );
		}
	}

	void Start()
	{
		std::cout << "서버 시작....." << std::endl;

		PostAccept();
	}

	void CloseSession( const int nSessionID )
	{
		std::cout << "클라이언트 접속 종료. 세션 ID: " << nSessionID << std::endl;

		m_SessionQueue.push_back( nSessionID );

		if( m_bIsAccepting == false )
		{
			PostAccept();
		}
	}

	void ProcessPacket( const int nSessionID, const char*pData )
	{
		PACKET_HEADER* pheader = (PACKET_HEADER*)pData;

		switch( pheader->nID )
		{
		case REQ_IN:
			{
				PKT_REQ_IN* pPacket = (PKT_REQ_IN*)pData;
				m_SessionList[ nSessionID ]->SetName( pPacket->szName );

				std::cout << "클라이언트 로그인 성공 Name: " << m_SessionList[ nSessionID ]->GetName() << std::endl; 

				PKT_RES_IN SendPkt;
				SendPkt.Init();
				SendPkt.bIsSuccess = true;
				
				m_SessionList[ nSessionID ]->PostSend( false, SendPkt.nSize, (char*)&SendPkt ); 
			}
			break;
		case REQ_CHAT:
			{
				PKT_REQ_CHAT* pPacket = (PKT_REQ_CHAT*)pData;

				PKT_NOTICE_CHAT SendPkt;
				SendPkt.Init();
				strncpy_s( SendPkt.szName, MAX_NAME_LEN, m_SessionList[ nSessionID ]->GetName(), MAX_NAME_LEN-1 );
				strncpy_s( SendPkt.szMessage, MAX_MESSAGE_LEN, pPacket->szMessage, MAX_MESSAGE_LEN-1 );

				size_t nTotalSessionCount = m_SessionList.size();
				
				for( size_t i = 0; i < nTotalSessionCount; ++i )
				{
					if( m_SessionList[ i ]->Socket().is_open() )
					{
						m_SessionList[ i ]->PostSend( false, SendPkt.nSize, (char*)&SendPkt ); 
					}
				}
			}
			break;
		}

		return;
	}

	bool SendInternal(int ownerSerial, char* buffer, unsigned int bufferSize)
	{
		Session* pSession = FindSession(ownerSerial);
		if(pSession)
		{
			if( pSession->Socket().is_open() )					
			{						
				pSession->SendInternal(buffer, bufferSize);
				return true;
			}
		}

		return false;
	}

	Session* FindSession(unsigned int sessionId)
	{
		size_t nTotalSessionCount = m_SessionList.size();
								
		for( size_t i = 0; i < nTotalSessionCount; ++i )				
		{					
			if( m_SessionList[ i ]->GetSessionId() == sessionId)						
				return m_SessionList[ i ];				
		}

		return 0;
	}

private:
	bool PostAccept()
	{
		if( m_SessionQueue.empty() )
		{
			m_bIsAccepting = false;
			return false;
		}
				
		m_bIsAccepting = true;
		int nSessionID = m_SessionQueue.front();

		m_SessionQueue.pop_front();
		
		m_acceptor.async_accept( m_SessionList[nSessionID]->Socket(),
								 boost::bind(&ChatServer::handle_accept, 
												this, 
												m_SessionList[nSessionID],
												boost::asio::placeholders::error)
								);

		return true;
	}

	void handle_accept(Session* pSession, const boost::system::error_code& error)
	{
		if (!error)
		{	
			std::cout << "클라이언트 접속 성공. SessionID: " << pSession->SessionID() << std::endl;
			
			pSession->Init();
			pSession->OnConnect(pSession->SessionID());

			pSession->PostReceive();
			
			PostAccept();
		}
		else 
		{
			std::cout << "error No: " << error.value() << " error Message: " << error.message() << std::endl;
		}
	}


	int m_nSeqNumber;
	
	bool m_bIsAccepting;

	boost::asio::ip::tcp::acceptor m_acceptor;

	std::vector< Session* > m_SessionList;
	std::deque< int > m_SessionQueue;

	INetworkEngine* m_pOwner;
	
};
