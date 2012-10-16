#pragma once
#include "SFPacketAnalyzer.h"

class SFUserSession
{
public:
	SFUserSession(int Serial);
	virtual ~SFUserSession(void);

	bool ProcessData(char* pData, unsigned short Length);

protected:

private:
	int m_Serial;
	SFPacketAnalyzer<SFCompressLzf> m_BufferAnalyzer;
};

