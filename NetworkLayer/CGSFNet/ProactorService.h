#pragma once
#include <ace/Asynch_IO.h>
#include <ACE/OS.h>
#include <ACE/Message_Block.h>
#include "InterlockedValue.h"

#include <EngineInterface/ISession.h>

class BaseClass;

class ProactorService : public ACE_Service_Handler, public ISession
{
public:
	ProactorService(int acceptorId = 0);
	virtual ~ProactorService(void);

	virtual void open(ACE_HANDLE h, ACE_Message_Block& MessageBlock) override;
	virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result& Result) override;
	virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result& Result) override;
	virtual void handle_time_out (const ACE_Time_Value &tv, const void *act = 0) override;

	void PostRecv();
	
	void SendInternal(char* pBuffer, int BufferSize, int ownerSerial = -1) override;	
	void Disconnect();

	void SetSerial(int serial){m_serial = serial;}
	int GetSerial(){ return m_serial; }

protected:
	void RegisterTimer();
	void UnregisterTimer();
	void ReserveClose();

private:	
	ACE_Asynch_Write_Stream m_AsyncWriter;
	ACE_Asynch_Read_Stream m_AsyncReader;

	int m_serial;
	int m_acceptorId;
	InterlockedValue* m_pTimerLock;

	bool m_bServiceCloseFlag;
};