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
		m_pTimerLock = 0;
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

	m_Serial = ProactorServiceMapSingleton::instance()->Register(this);

	assert(m_Serial != INVALID_ID);

	RegisterTimer();

	ISession::OnConnect(this->m_Serial);

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
		return;
	}
}

void ProactorService::handle_read_stream( const ACE_Asynch_Read_Stream::Result& Result )
{
	ACE_Message_Block& Block = Result.message_block();
	if(!Result.success() || Result.bytes_transferred() == 0)
	{
		Block.release();
		ReserveClose();
	}
	else
	{
		if(false == ISession::OnReceive(Block.rd_ptr(), Block.length()))
		{
			Block.release();
			ReserveClose();
			return;
		}

		PostRecv();
	}
}

void ProactorService::handle_write_stream( const ACE_Asynch_Write_Stream::Result& Result )
{
	Result.message_block().release();
}

void ProactorService::RegisterTimer()
{
	ACE_Time_Value intervalTime(5,0);
	ACE_Proactor::instance()->schedule_timer(*this, 0, ACE_Time_Value::zero, intervalTime);
}

void ProactorService::UnregisterTimer()
{
	ProactorServiceMapSingleton::instance()->UnRegister(m_Serial);
}

void ProactorService::ReserveClose()
{	
	if(this->handle() != ACE_INVALID_HANDLE)
		ACE_OS::closesocket(this->handle());
	
	this->handle(ACE_INVALID_HANDLE);	

	UnregisterTimer();
	ProactorServiceMapSingleton::instance()->UnRegister(m_Serial);

	ISession::OnDisconnect(this->m_Serial);

	m_bServiceCloseFlag = true;
}

void ProactorService::handle_time_out(const ACE_Time_Value& tv, const void* arg)
{
	ACE_UNUSED_ARG(tv);
	ACE_UNUSED_ARG(arg);

	if(m_bServiceCloseFlag == true && m_pTimerLock->Acquire() == true)
	{		
		ACE_Proactor::instance()->cancel_timer(*this);			
		delete this;	
	}
}

void ProactorService::SendInternal(char* pBuffer, int BufferSize, int ownerSerial)
{			
	ACE_Message_Block* pBlock = NULL;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block (BufferSize));

	pBlock->copy((const char*)pBuffer, BufferSize);

	if(NULL == pBlock->cont())
	{
		m_AsyncWriter.write(*pBlock, pBlock->length());
	}
	else
	{
		m_AsyncWriter.writev(*pBlock, pBlock->total_length());
	}	
}