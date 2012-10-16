#pragma once
#include <ace/Asynch_IO.h>
#include <ACE/OS.h>
#include <ACE/Message_Block.h>

class INetworkEngine;

class ProactorService : public ACE_Service_Handler
{
public:
	ProactorService();
	virtual ~ProactorService(void);

	virtual void open(ACE_HANDLE h, ACE_Message_Block& MessageBlock) override;
	virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result& Result) override;
	virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result& Result) override;

	void PostRecv();

	BOOL Send(char* pBuffer, int BufferSize);

	void SetOwner(INetworkEngine* pOwner){m_pOwner = pOwner;}
	void SetSerial(int Serial){m_Serial = Serial;}
private:
	ACE_Asynch_Write_Stream m_AsyncWriter;
	ACE_Asynch_Read_Stream m_AsyncReader;

	INetworkEngine* m_pOwner;
	int m_Serial;
};