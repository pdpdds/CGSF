#include "StdAfx.h"
#include "SFList.h"

SFList::SFList()
{
	m_posHead = m_posTail = m_posUsed = NULL;
}

SFList::~SFList()
{
	while(m_posHead != NULL)
	{
		OPOS pos = m_posHead;
		m_posHead = m_posHead->m_posNext;
		delete pos;
	}

	while(m_posUsed != NULL)
	{
		OPOS pos = m_posUsed;
		m_posUsed = m_posUsed->m_posNext;
		delete pos;
	}
}

OPOS SFList::_NewNode()
{
	OPOS node;
	if(m_posUsed == NULL)
		node = new _node;
	else
	{
		node = m_posUsed;
		m_posUsed = m_posUsed->m_posNext;
	}

	node->m_posNext = NULL;
	node->m_posPrev = NULL;

	return node;
}

void SFList::_DelNode(OPOS Node)
{
	if(Node->m_posPrev)
		Node->m_posPrev->m_posNext = Node->m_posNext;
	if(Node->m_posNext)
		Node->m_posNext->m_posPrev = Node->m_posPrev;

	Node->m_posNext = m_posUsed;
	m_posUsed = Node;
}

BOOL SFList::IsEmpty()
{
	return (m_posHead == NULL && m_posTail == NULL);
}

int SFList::GetCount()
{
	OPOS pos = m_posHead;
	int n = 0;

	for(; pos != NULL; n++)
		pos = pos->m_posNext;

	return n;
}

OPOS SFList::Findindex(int nIndex)
{
	OPOS pos = m_posHead;

	for(int i = 0; i < nIndex && pos != NULL; i++)
		pos = pos->m_posNext;

	return pos;
}

OPOS SFList::Find(void* pObject, OPOS posAfter)
{
	OPOS pos = (posAfter == NULL) ? m_posHead : posAfter;

	while(pos != NULL)
	{
		if(pos->m_pObject == pObject)
			return pos;

		pos = pos->m_posNext;
	}

	return NULL;
}

OPOS SFList::InsertAfter(OPOS pos, void* pObject)
{
	if(pos == NULL)
		return NULL;

	OPOS posNew = _NewNode();
	posNew->m_pObject = pObject;
	posNew->m_posPrev = pos;

	posNew->m_posNext = pos->m_posNext;

	posNew->m_posNext = posNew;

	if(posNew->m_posNext == NULL)
		m_posTail = posNew;
	else
		posNew->m_posNext->m_posPrev = posNew;

	return posNew;
}

OPOS SFList::InsertBefore(OPOS pos, void* pObject)
{
	if(pos == NULL)
		return NULL;

	OPOS posNew = _NewNode();
	posNew->m_pObject = pObject;
	posNew->m_posNext = pos;
	posNew->m_posPrev = pos->m_posPrev;

	posNew->m_posPrev = posNew;

	if(posNew->m_posPrev == NULL)
		m_posHead = posNew;
	else
		posNew->m_posPrev->m_posNext = posNew;

	return posNew;
}

void* SFList::RemoveAt(OPOS pos)
{
	if(pos == NULL)
		return NULL;

	if(m_posHead == pos)
		m_posHead = pos->m_posNext;

	if(m_posTail == pos)
		m_posTail = pos->m_posPrev;

	void* pObject = pos->m_pObject;
	_DelNode(pos);

	return pObject;
}

void SFList::SetAt(OPOS pos, void* pObject)
{
	if(pos == NULL)	
		return;

	pos->m_pObject = pObject;
}

void* SFList::GetAt(OPOS pos)
{
	if(pos == NULL)
		return NULL;

	return pos->m_pObject;
}

void* SFList::GetPrev(OPOS& pos)
{
	if(pos == NULL)
		return NULL;

	void* pObject = pos->m_pObject;
	pos = pos->m_posPrev;

	return pObject;
}

void* SFList::GetNext(OPOS& pos)
{
	if(pos == NULL)
		return NULL;

	void* pObject = pos->m_pObject;
	pos = pos->m_posNext;
	return pObject;
}

OPOS SFList::GetTailPosition()
{
	return m_posTail;
}

OPOS SFList::GetHeadPosition()
{
	return m_posHead;
}

void SFList::RemoveAll()
{
	while(m_posHead != NULL)
	{
		OPOS pos = m_posHead->m_posNext;
		_DelNode(m_posHead);
		m_posHead = pos;
	}

	m_posTail = NULL;
}

OPOS SFList::AddTail(void* pObject)
{
	OPOS posNew = _NewNode();

	if(posNew == NULL)
		return NULL;

	posNew->m_posPrev = m_posTail;
	posNew->m_pObject = pObject;
	m_posTail = posNew;

	if(posNew->m_posPrev == NULL)
		m_posHead = m_posTail;
	else
		posNew->m_posPrev->m_posNext = posNew;

	return posNew;
}

OPOS SFList::AddHead(void* pObject)
{
	OPOS posNew = _NewNode();

	if(posNew == NULL)
		return NULL;

	posNew->m_posNext = m_posHead;
	posNew->m_pObject = pObject;
	m_posHead = posNew;

	if(posNew->m_posNext == NULL)
		m_posTail = m_posHead;
	else
		posNew->m_posNext->m_posPrev = posNew;

	return posNew;
}

void* SFList::RemoveTail()
{
	if(m_posTail == NULL)
		return NULL;

	OPOS posTemp = m_posTail;

	m_posTail = m_posTail->m_posPrev;

	void* pObject = posTemp->m_pObject;

	_DelNode(posTemp);

	if(m_posTail == NULL)
		m_posHead = NULL;

	return pObject;
}

void* SFList::RemoveHead()
{
	if (m_posHead == NULL)
		return NULL;

	OPOS posTemp = m_posHead;
	m_posHead = m_posHead->m_posNext;

	void* pObject = posTemp->m_pObject;
	_DelNode(posTemp);

	if(m_posHead == NULL)
		m_posTail = NULL;

	return pObject;
}

void* SFList::GetTail()
{
	if (m_posTail == NULL)
		return NULL;

	return m_posTail->m_pObject;
}

void* SFList::GetHead()
{
	if (m_posHead == NULL)
		return NULL;

	return m_posHead->m_pObject;
}