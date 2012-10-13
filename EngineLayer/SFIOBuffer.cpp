#include "StdAfx.h"
#include "SFIOBuffer.h"

SFIOBuffer::SFIOBuffer(void)
: m_iBufSize(0)
, m_pBuffer(NULL)
, m_iUsedBufferSize(0)
{
}

SFIOBuffer::~SFIOBuffer(void)
{
	FreeIOBuf();
}

VOID SFIOBuffer::InitIOBuf()
{
	m_iTail = 0;
	m_iHead = 0;

	m_iUsedBufferSize = 0;

	if(m_pBuffer)
		memset(m_pBuffer, 0, sizeof(char) * m_iBufSize);
}

int SFIOBuffer::AppendData( const char* pBuffer, int iSize )
{ 
	if(m_iUsedBufferSize + iSize >= m_iBufSize)
	{
		return -1;
	}

	int iAddedData = 0;
	int iAdded = 0;

	while(iSize > 0)
	{
		if(iSize > m_iBufSize-m_iTail)
			iAddedData = m_iBufSize-m_iTail;
		else iAddedData = iSize;

		if(iAddedData)
		{
			memcpy(m_pBuffer + m_iTail, pBuffer, iAddedData);

			iAdded += iAddedData;

			iSize -= iAddedData;
			pBuffer += iAddedData;
			m_iTail += iAddedData;

			if(m_iTail >= m_iBufSize)
				m_iTail -= m_iBufSize;
		}
	}

	CalcUsedBufferSize();

	return iAdded;


}

int SFIOBuffer::GetData( char* pBuffer, int iSize )
{
	if(GetUsedBufferSize() < iSize)
		iSize = GetUsedBufferSize();

	if(iSize <= 0)
		return 0;

	if(m_iHead + iSize > m_iBufSize)
	{
		int iDiff = m_iBufSize - m_iHead;
		memcpy(pBuffer, m_pBuffer + m_iHead, iDiff);
		memcpy(pBuffer + iDiff, m_pBuffer, iSize - iDiff);
	}
	else
	{
		memcpy(pBuffer, m_pBuffer + m_iHead, iSize);
	}

	m_iHead += iSize;
	if(m_iHead >= m_iBufSize)
	{
		m_iHead -= m_iBufSize;
	}

	CalcUsedBufferSize();

	return iSize;
}

BOOL SFIOBuffer::AllocIOBuf( int iBufSize )
{
	if(iBufSize <= 0)
		iBufSize = MAX_IO_SIZE;

	if(iBufSize > MAX_IO_SIZE)
	{
		//SFLog
		return FALSE;
	}

	m_pBuffer = new char[iBufSize];
	if(m_pBuffer == NULL)
	{
		return FALSE;
	}

	m_iBufSize = iBufSize;

	InitIOBuf();

	return TRUE;
}

BOOL SFIOBuffer::FreeIOBuf()
{
	if(m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}

	m_iBufSize = 0;
	m_iHead = m_iTail = 0;
	m_iUsedBufferSize = 0;

	return TRUE;

}

void SFIOBuffer::CalcUsedBufferSize()
{
	if(m_iHead > m_iTail)
		m_iUsedBufferSize =  m_iBufSize - m_iHead + m_iTail;
	else
		m_iUsedBufferSize =  m_iTail - m_iHead;
}
