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
#define MEMCHUNK_DEFAULT_SIZE					(256)
#define MEMCHUNK_DEFAULT_EXTSIZE				(256)

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

////////////////////////////////////////////////////////////////////////////////
//
class CMemChunkNode: public CMemChunk, public CNode
{
public:
	CMemChunkNode();
	virtual ~CMemChunkNode();

public:
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual LONG CompareTo(CNode *pNode);

private:
	ULONG m_cRefs;
};

#endif
