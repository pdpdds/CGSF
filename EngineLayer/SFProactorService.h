#pragma once
#include <ace/Asynch_IO.h>
#include <ACE/OS.h>
#include <ACE/Message_Block.h>
#include "SFPacketAnalyzer.h"
//#include "SFPacketStore.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "SFCompressLzf.h"
#include "SFCompressZLib.h"
//#include "loki/smallobj.h"

//#pragma comment(lib, "loki_d.lib")
//SFProactorService
class IProactorAgent;

using namespace google;

class SFProactorService : public ACE_Service_Handler
{
public:
	SFProactorService();
	virtual ~SFProactorService(void);

	virtual void open(ACE_HANDLE h, ACE_Message_Block& MessageBlock) override;
	virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result& Result) override;
	virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result& Result) override;

	BOOL Send(USHORT PacketID, char* pBuffer, int BufferSize);
	BOOL Send(SFPacket* pPacket);

	void SetOwner(IProactorAgent* pOwner){m_pOwner = pOwner;}
	void SetSerial(int Serial){m_Serial = Serial;}
private:
	ACE_Asynch_Write_Stream m_AsyncWriter;
	ACE_Asynch_Read_Stream m_AsyncReader;
	SFPacketAnalyzer<SFCompressLzf> m_BufferAnalyzer;

	IProactorAgent* m_pOwner;
	int m_Serial;
};
