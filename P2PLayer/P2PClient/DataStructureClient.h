////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.03.07
// 설  명 :
//

#ifndef __DATASTRUCTURECLIENT_H__
#define __DATASTRUCTURECLIENT_H__

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

#endif

