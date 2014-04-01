#ifndef IOBUFFER_H
#define IOBUFFER_H
#include <memory>

class IOBuffer
{
#define MAX_IO_SIZE 16384

public:
	IOBuffer();
	virtual ~IOBuffer();

	void	InitIOBuf();
	bool	AllocIOBuf(int iBufSize);
	bool	FreeIOBuf();

	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////

	int		AppendData(const char* pBuffer, int iSize);
	int		GetData(char* pBuffer, int iSize);

	///////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////

	inline char* GetBuffer() { return	m_pBuffer; }
	inline int	GetBufSize() { return m_iBufSize; }

	inline int GetUsedBufferSize() { return m_iUsedBufferSize; }
	inline int GetEmptyBufferSize() { return m_iBufSize - m_iUsedBufferSize; }

	void	CalcUsedBufferSize();


private:
	int		m_iHead;
	int		m_iTail;

	int		m_iBufSize;

	char*	m_pBuffer;

	int		m_iUsedBufferSize;
};

#endif