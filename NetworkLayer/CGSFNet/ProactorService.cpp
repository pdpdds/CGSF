#include "ProactorService.h"
#include "SingltonObject.h"
#include <Assert.h>

ProactorService::ProactorService()
	: m_bServiceCloseFlag(false)
	, m_pTimerLock(0)
{
}

ProactorService::~ProactorService( void )
{	
	if(m_pTimerLock)
	{
		delete m_pTimerLock;
		m_pTimerLock = NULL;
	}
}

void ProactorService::open( ACE_HANDLE h, ACE_Message_Block& MessageBlock )
{
	this->handle(h);	

	if (this->m_AsyncReader.open(*this) != 0 || this->m_AsyncWriter.open(*this) != 0)
	{
		delete this;
		return;
	}
	
	m_pTimerLock = new InterlockedValue();
	m_pTimerLock->Release();

	m_bServiceCloseFlag = false;

	m_serial = ProactorServiceManagerSinglton::instance()->Register(this);

	assert(m_serial != INVALID_ID);

	RegisterTimer();

	ISession::OnConnect(this->m_serial, m_sessionDesc);

	PostRecv();
}

void ProactorService::PostRecv()
{
	ACE_Message_Block* pBlock;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block (2048));
	if(this->m_AsyncReader.read(*pBlock, pBlock->space()) != 0)
	{
		pBlock->release();
		ReserveClose();
	}
}

void ProactorService::handle_read_stream( const ACE_Asynch_Read_Stream::Result& result )
{
	ACE_Message_Block& block = result.message_block();
	if (!result.success() || result.bytes_transferred() == 0)
	{
		block.release();
		ReserveClose();
	}
	else
	{
		if (false == ISession::OnReceive(block.rd_ptr(), (unsigned short)block.length(), m_sessionDesc))
		{
			block.release();
			ReserveClose();
			return;
		}

		PostRecv();
	}
}

void ProactorService::handle_write_stream( const ACE_Asynch_Write_Stream::Result& result )
{
	result.message_block().release();
}

void ProactorService::RegisterTimer()
{
	ACE_Time_Value intervalTime(5,0);
	ACE_Proactor::instance()->schedule_timer(*this, 0, ACE_Time_Value::zero, intervalTime);
}

void ProactorService::UnregisterTimer()
{
	ACE_Proactor::instance()->cancel_timer(*this);
}

void ProactorService::Disconnect()
{
	if (this->handle() != ACE_INVALID_HANDLE)
		ACE_OS::closesocket(this->handle());

	this->handle(ACE_INVALID_HANDLE);

	m_bServiceCloseFlag = true;
}

void ProactorService::ReserveClose()
{	
	ProactorServiceManagerSinglton::instance()->UnRegister(m_serial);

	if(this->handle() != ACE_INVALID_HANDLE)
		ACE_OS::closesocket(this->handle());
	
	this->handle(ACE_INVALID_HANDLE);	

	ISession::OnDisconnect(this->m_serial, m_sessionDesc);

	m_bServiceCloseFlag = true;
}

void ProactorService::handle_time_out(const ACE_Time_Value& tv, const void* arg)
{
	ACE_UNUSED_ARG(tv);
	ACE_UNUSED_ARG(arg);

	if(m_bServiceCloseFlag == true && m_pTimerLock->Acquire() == true)
	{			
		UnregisterTimer();
		delete this;	
	}
}

void ProactorService::SendInternal(char* pBuffer, int bufferSize)
{			
	ACE_Message_Block* pBlock = NULL;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block(bufferSize));

	pBlock->copy((const char*)pBuffer, bufferSize);

	if(NULL == pBlock->cont())
	{
		m_AsyncWriter.write(*pBlock, pBlock->length());
	}
	else
	{
		m_AsyncWriter.writev(*pBlock, pBlock->total_length());
	}	
}

bool ProactorService::SendRequest(BasePacket* pPacket)
{
	IPacketProtocol* pProtocol = GetPacketProtocol();
	if (NULL == pProtocol)
		return false;

	char* pBuffer = NULL;
	int bufferSize = 0;
	if (false == pProtocol->Encode(pPacket, &pBuffer, bufferSize))
		return false;

	SendInternal(pBuffer, bufferSize);

	return true;
}