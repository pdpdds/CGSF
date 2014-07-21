#pragma once
#include <ace/Asynch_IO.h>
#include <ACE/OS.h>
#include <ACE/Message_Block.h>
#include "InterlockedValue.h"

#include "EngineInterface/EngineStructure.h"
#include "BasePacket.h"

#include <EngineInterface/ISession.h>

class ProactorService : public ACE_Service_Handler, public ISession
{
public:
	ProactorService();
	virtual ~ProactorService(void);

	virtual void open(ACE_HANDLE h, ACE_Message_Block& MessageBlock) override;
	virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result& Result) override;
	virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result& Result) override;
	virtual void handle_time_out (const ACE_Time_Value &tv, const void *act = 0) override;

	void PostRecv();
	
	virtual bool SendRequest(BasePacket* pPacket) override;
	void SendInternal(char* pBuffer, int BufferSize);	
	void Disconnect();

	void SetSerial(int serial){m_serial = serial;}
	int GetSerial(){ return m_serial; }

	void SetSessionDesc(_SessionDesc& desc){ m_sessionDesc = desc; };

protected:
	void RegisterTimer();
	void UnregisterTimer();
	void ReserveClose();

private:	
	ACE_Asynch_Write_Stream m_AsyncWriter;
	ACE_Asynch_Read_Stream m_AsyncReader;

	int m_serial;
	_SessionDesc m_sessionDesc;
	InterlockedValue* m_pTimerLock;

	bool m_bServiceCloseFlag;
};
