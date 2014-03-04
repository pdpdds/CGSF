#pragma once

typedef struct _node
{
	_node* m_posNext;
	_node* m_posPrev;
	void* m_pObject;
} *OPOS;

class SFList
{
public:
		
	SFList();
	virtual ~SFList();

	BOOL IsEmpty();
	int GetCount();
	OPOS Findindex(int nIndex);
	OPOS Find(void* pObject, OPOS posAfter = NULL);
	OPOS InsertAfter(OPOS pos, void* pObject);
	OPOS InsertBefore(OPOS pos, void* pObject);
	void* RemoveAt(OPOS pos);
	void SetAt(OPOS pos, void* pObject);
	void* GetAt(OPOS pos);
	void* GetPrev(OPOS& pos);
	void* GetNext(OPOS& pos);
	OPOS GetTailPosition();
	OPOS GetHeadPosition();
	void RemoveAll();
	OPOS AddTail(void* pObject);
	OPOS AddHead(void* pObject);
	void* RemoveTail();
	void* RemoveHead();
	void* GetTail();
	void* GetHead();

protected:

private:
	OPOS _NewNode();
	void _DelNode(OPOS Node);

	OPOS m_posTail;
	OPOS m_posHead;
	OPOS m_posUsed;
};