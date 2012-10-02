////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.01.28
// 설  명 :
//

#ifndef __DATAST_H__
#define __DATAST_H__

////////////////////////////////////////////////////////////////////////////////
//
class CNode 
{
public:
	CNode()
	{
		m_pcPrev = NULL; 
		m_pcNext = NULL; 
	}
	virtual ~CNode() {}

public:
	virtual ULONG AddRef() = 0;
	virtual ULONG Release() = 0;
	virtual LONG CompareTo(CNode *pcNode) = 0;

public:
	inline CNode* GetPrev() { return m_pcPrev; }
	inline CNode* GetNext() { return m_pcNext; }
	inline void SetPrev(CNode *pcPrev) { m_pcPrev = pcPrev; }
	inline void SetNext(CNode *pcNext) { m_pcNext = pcNext; }
	inline void Reset() 
	{ 
		m_pcPrev = NULL; 
		m_pcNext = NULL; 
	}

private:
	CNode *m_pcPrev;
	CNode *m_pcNext;
};

////////////////////////////////////////////////////////////////////////////////
//
typedef CNode *PCNode;

////////////////////////////////////////////////////////////////////////////////
//
class CBoList
{
public:
	CBoList();
	virtual ~CBoList();

public:
	void AddAtFirst(PCNode pcNode);
	void Add(PCNode pcNode);
	void Delete(PCNode pcNode);
	void DeletePtr(PCNode pcNode);
	PCNode DelWithLive(PCNode pcNode);
	PCNode DelWithLiveInPtr(PCNode pcDelNodePtr);
	PCNode DelBeginWithLive();
	PCNode DelEndWithLive();
	PCNode Find(const PCNode pcNode);
	void Reset();

public:
	inline PCNode GetBegin() { return m_pcBegin; }
	inline PCNode GetEnd() { return m_pcEnd; }
	inline BOOL IsEmpty() { return ((NULL == m_pcBegin)?TRUE:FALSE); }

private:
	PCNode m_pcBegin;
	PCNode m_pcEnd;
};

////////////////////////////////////////////////////////////////////////////////
//
class CTreeNode: public CNode
{
public:
	CTreeNode()
	{
		m_pcLeft = NULL;
		m_pcRight = NULL;
		m_pcParent = NULL;
	}
	virtual ~CTreeNode() {}

public:
	inline CTreeNode* GetLeft() { return m_pcLeft; }
	inline CTreeNode* GetRight() { return m_pcRight; }
	inline CTreeNode* GetParent() { return m_pcParent; }
	inline void SetLeft(CTreeNode* pLeft) { m_pcLeft = pLeft; }
	inline void SetRight(CTreeNode* pRight) { m_pcRight = pRight; }
	inline void SetParent(CTreeNode* pParent) { m_pcParent = pParent; }
	inline void Reset() 
	{ 
		m_pcParent = NULL; 
		m_pcLeft = NULL; 
		m_pcRight = NULL; 

		CNode::Reset(); 
	}

private:
	CTreeNode* m_pcParent;
	CTreeNode* m_pcLeft;
	CTreeNode* m_pcRight;
};

////////////////////////////////////////////////////////////////////////////////
//
typedef CTreeNode *PCTreeNode;

////////////////////////////////////////////////////////////////////////////////
//
class CBtree
{
public:
	CBtree();
	virtual ~CBtree();

	BOOL Add(PCTreeNode pTreeNode);
	BOOL Delete(PCTreeNode pTreeNode);
	PCTreeNode DelWithLive(PCTreeNode pTreeNode);
	PCTreeNode Find(const PCTreeNode pTreeNode);
	void Reset();

public:
	inline PCTreeNode GetFirst() { return m_pcFirst; }
	inline PCTreeNode GetLast() { return m_pcLast; }
	inline PCTreeNode GetRoot() { return m_pcRoot; }
	inline BOOL IsEmpty() { return ((NULL==m_pcRoot)?TRUE:FALSE); }

protected:
	PCTreeNode GetLeftEnd(PCTreeNode pNode);
	PCTreeNode GetRightEnd(PCTreeNode pNode);

private:
	void DelNodeAndLink(PCTreeNode pTreeNode);

private:
	PCTreeNode m_pcRoot;
	PCTreeNode m_pcFirst;
	PCTreeNode m_pcLast;
};

////////////////////////////////////////////////////////////////////////////////
//
typedef CBtree *PCBtree;

////////////////////////////////////////////////////////////////////////////////
//
template<class T>
class CQueue
{
public:
	CQueue();
	virtual ~CQueue();

public:
	BOOL Init();
	void Uninit();
	BOOL Push(T tData);
	BOOL Pop(T* ptData);

public:
	inline void Clear() { m_lHead = m_lTail = 0; }
	inline BOOL Empty() { return ((m_lHead == m_lTail)?TRUE:FALSE); }
	inline void SetSize(LONG lSize) { m_lMaxSize = lSize + 1; }
	inline LONG GetSize() { return m_lMaxSize; }

private:
	LONG m_lMaxSize;
	LONG m_lHead;
	LONG m_lTail;

private:
	T* m_pQueueData;
};

/////////////////////////////////////////////////////////////////////////////////////////
// 
template<class T>
CQueue<T>::CQueue()
{
	m_lMaxSize = 0;
	m_lHead = 0;
	m_lTail = 0;
	m_pQueueData = NULL;
}

template<class T>
CQueue<T>::~CQueue()
{
	if (m_pQueueData)
	{
		delete[] m_pQueueData;
		m_pQueueData = NULL;
	}
}

template<class T>
BOOL 
CQueue<T>::Init()
{
	if (0 >= m_lMaxSize)
	{
		return FALSE;
	}

	m_pQueueData = new T[m_lMaxSize];
	if (NULL == m_pQueueData)
	{
		return FALSE;
	}

	m_lHead = 0;
	m_lTail = 0;

	return TRUE;
}

template<class T>
void 
CQueue<T>::Uninit()
{
	if (m_pQueueData)
	{
		delete[] m_pQueueData;
		m_pQueueData = NULL;
	}

	m_lHead = 0;
	m_lTail = 0;
}

template<class T>
BOOL 
CQueue<T>::Push(T tData)
{
	LONG lTmpHead = (m_lHead + 1) % m_lMaxSize;

	if (lTmpHead == m_lTail)
	{
		return FALSE;
	}

	m_pQueueData[m_lHead] = tData;
	m_lHead = lTmpHead;

	return TRUE;
}

template<class T>
BOOL
CQueue<T>::Pop(T* ptData)
{
	if (m_lHead == m_lTail)
	{
		return FALSE;
	}

	*ptData = m_pQueueData[m_lTail];
	m_lTail = (m_lTail + 1) % m_lMaxSize;

	return TRUE;
}

#endif

