////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 
// 설  명 :
//

#ifndef __MEMCHUNK_H__
#define __MEMCHUNK_H__

////////////////////////////////////////////////////////////////////////////////
//
#define MEMCHUNK_DEFAULT_SIZE					(2048)
#define MEMCHUNK_DEFAULT_EXTSIZE				(1024)

////////////////////////////////////////////////////////////////////////////////
//
class CMemChunk
{
public:
	CMemChunk();
	virtual ~CMemChunk();

	BOOL Init(LONG lSize = MEMCHUNK_DEFAULT_SIZE, LONG lExtSize = MEMCHUNK_DEFAULT_EXTSIZE);
	void Uninit();
	BOOL Ext(LONG lSize);

public:
	inline void* GetMemChunk() { return m_pMem; }
	inline LONG GetMemChunkSize() { return m_lMemSize; } 

private:
	LONG m_lMemSize;
	LONG m_lExtSize;
	void* m_pMem;
};

#endif

