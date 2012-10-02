#include "StdAfx.h"
#include "SFProactorService.h"
#include "SFCompressZlib.h"
#include "IProactorAgent.h"

SFProactorService::SFProactorService()
{

}

SFProactorService::~SFProactorService( void )
{
	if(this->handle() != ACE_INVALID_HANDLE)
		ACE_OS::closesocket(this->handle());

	m_pOwner->ServiceDisconnect(m_Serial);
}

void SFProactorService::open( ACE_HANDLE h, ACE_Message_Block& MessageBlock )
{
	this->handle(h);

	if (this->m_AsyncReader.open(*this) != 0 || this->m_AsyncWriter.open(*this) != 0)
	{
		SFASSERT(0);
		delete this;
		return;
	}

	m_pOwner->ServiceInitialize(this);

	ACE_Message_Block* pBlock;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block (2048));
	if(this->m_AsyncReader.read(*pBlock, pBlock->space()) != 0)
	{
		pBlock->release();
		delete this;
		return;
	}

	return;
}

void SFProactorService::handle_read_stream( const ACE_Asynch_Read_Stream::Result& Result )
{
	ACE_Message_Block& Block = Result.message_block();
	if(!Result.success() || Result.bytes_transferred() == 0)
	{
		Block.release();
		delete this;
	}
	else
	{
		m_BufferAnalyzer.AddTransferredData(Block.rd_ptr(), Block.length());
		
		int ErrorCode = PACKETIO_ERROR_NONE;

		SFPacket* pPacket = PacketPoolSingleton::instance()->Alloc();

		while(TRUE ==  m_BufferAnalyzer.GetPacket(*pPacket, ErrorCode))
		{
			pPacket->SetPacketType(SFCommand_Data);
			pPacket->SetOwnerSerial(m_Serial);
			
			LogicGatewaySingleton::instance()->PushPacket(pPacket);

			pPacket = PacketPoolSingleton::instance()->Alloc();
			pPacket->Initialize();
		}

		 PacketPoolSingleton::instance()->Release(pPacket);

		if(ErrorCode != PACKETIO_ERROR_NONE)
		{
			Block.release();
			delete this;
			return;
		}

		ACE_Message_Block* pNewBlock;
		ACE_NEW_NORETURN(pNewBlock, ACE_Message_Block(2048));
		this->m_AsyncReader.read(*pNewBlock, pNewBlock->space());
	}
}

void SFProactorService::handle_write_stream( const ACE_Asynch_Write_Stream::Result& Result )
{
	Result.message_block().release();
}


BOOL SFProactorService::Send(USHORT PacketID, char* pBuffer, int BufferSize)
{
	int HeaderSize = sizeof(SFPacketHeader);

	SFPacket PacketSend;

	PacketSend.SetPacketID(PacketID);

	PacketSend.MakePacket((BYTE*)pBuffer, BufferSize, CGSF_PACKET_OPTION);

	ACE_Message_Block* pBlock = NULL;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block (PacketSend.GetPacketSize()));

	SFASSERT(PacketSend.GetHeaderSize() + PacketSend.GetDataSize() == PacketSend.GetPacketSize());

	pBlock->copy((const char*)PacketSend.GetHeader(), PacketSend.GetHeaderSize());

	if(PacketSend.GetDataSize() != 0)
		pBlock->copy((const char*)PacketSend.GetDataBuffer(), PacketSend.GetDataSize());

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


BOOL SFProactorService::Send(SFPacket* pPacket)
{
	ACE_Message_Block* pBlock = NULL;

	ACE_NEW_NORETURN(pBlock, ACE_Message_Block (pPacket->GetPacketSize()));

	SFASSERT(pPacket->GetHeaderSize() + pPacket->GetDataSize() == pPacket->GetPacketSize());

	pBlock->copy((const char*)pPacket->GetHeader(), pPacket->GetHeaderSize());

	if(pPacket->GetDataSize() != 0)
		pBlock->copy((const char*)pPacket->GetDataBuffer(), pPacket->GetDataSize());

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