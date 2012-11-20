#pragma once
#include "SFPacketAnalyzer.h"

class SFUserSession
{
public:
	SFUserSession();
	virtual ~SFUserSession(void);

	bool ProcessData(int Serial, char* pData, unsigned short Length);

protected:

private:
	SFPacketAnalyzer<SFCompressLzf> m_BufferAnalyzer;
};

