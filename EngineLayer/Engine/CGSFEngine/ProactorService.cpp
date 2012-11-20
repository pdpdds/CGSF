#include "ProactorService.h"
#include "SingltonObject.h"
#include <Assert.h>

ProactorService::ProactorService()
{
}

ProactorService::~ProactorService( void )
{
	if(this->handle() != ACE_INVALID_HANDLE)
		ACE_OS::closesocket(this->handle());

	ProactorServiceMapSingleton::instance()->UnRegister(m_Serial);

	OnDisconnect(this->m_Serial);
}

void ProactorService::open( ACE_HANDLE h, ACE_Message_Block& MessageBlock )
{
	this->handle(h);

	if (this->m_AsyncReader.open(*this) != 0 || this->m_AsyncWriter.open(*this) != 0)
	{
		delete this;
		return;
	}

	m_Serial = ProactorServiceMapSingleton::instance()->Register(this);

	assert(m_Serial != INVALID_ID);

	OnConnect(this->m_Serial);

	PostRecv();

	return;
}


void ProactorService::PostRecv()
{
	ACE_Message_Block* pBlock;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block (2048));
	if(this->m_AsyncReader.read(*pBlock, pBlock->space()) != 0)
	{
		pBlock->release();
		delete this;
		return;
	}
}

void ProactorService::handle_read_stream( const ACE_Asynch_Read_Stream::Result& Result )
{
	ACE_Message_Block& Block = Result.message_block();
	if(!Result.success() || Result.bytes_transferred() == 0)
	{
		Block.release();
		delete this;
	}
	else
	{
		if(false == OnData(m_Serial, Block.rd_ptr(), Block.length()))
		{
			Block.release();
			delete this;
			return;
		}

		PostRecv();
	}
}

void ProactorService::handle_write_stream( const ACE_Asynch_Write_Stream::Result& Result )
{
	Result.message_block().release();
}

BOOL ProactorService::Send(char* pBuffer, int BufferSize)
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

	return TRUE;
}