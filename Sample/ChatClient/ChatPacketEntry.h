#pragma once
#include "ILogicEntry.h"

class SFCommand;

class ChatPacketEntry : public ILogicEntry
{
public:
	ChatPacketEntry(void);
	virtual ~ChatPacketEntry(void);

	virtual BOOL ProcessPacket(BasePacket* pPacket) override;

	BOOL SendRequest(BasePacket* pPacket);

	int GetSerial(){return m_Serial;}

protected:

private:
	int m_Serial;
};

/*m_pPacketProtocol->

		char* pBuffer = m_pPacketProtocol->GetData();
		int BufferSize = m_pPacketProtocol->GetSize();

		if (oBuffer.GetDataSize()==0){
			return true;

			SendInternal(pBuffer, BufferSize);

			return true;
		}

	unsigned int uSize = min(4096, oBuffer.GetDataSize());

	
		oBuffer.Pop(uSize);
		return WriteBytes();*/