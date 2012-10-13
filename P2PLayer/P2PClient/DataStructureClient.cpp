////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.01.28
// 설  명 :
//

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfClientSys.h"
#include "AllOcfClient.h"

/////////////////////////////////////////////////////////////////////////////////////////
// 
CBoList::CBoList():
	m_pcBegin(NULL),
	m_pcEnd(NULL) 
{
}

CBoList::~CBoList() 
{
	CNode *pcTmp;
	PCNode pcCurrent = GetBegin();

	while(pcCurrent)
	{
		pcTmp = pcCurrent->GetNext();
		pcCurrent->Release();

		pcCurrent = pcTmp;
	}
}

void 
CBoList::AddAtFirst(PCNode pcNode)
{
	pcNode->Reset();

	if (NULL == m_pcBegin && 
		NULL == m_pcEnd)
	{
		m_pcBegin = pcNode;
		m_pcEnd = pcNode;
	}
	else
	{
		pcNode->SetNext(m_pcBegin);
		m_pcBegin->SetPrev(pcNode);
		m_pcBegin = pcNode;
	}
}

void
CBoList::Add(PCNode pcNode)
{
	pcNode->Reset();

	if (NULL == m_pcBegin && 
		NULL == m_pcEnd)
	{
		m_pcBegin = pcNode;
		m_pcEnd = pcNode;
	}
	else
	{
		m_pcEnd->SetNext(pcNode);
		pcNode->SetPrev(m_pcEnd);
		m_pcEnd = pcNode;
	}
}

void
CBoList::Delete(PCNode pcNode)
{
	PCNode pcDelNode = DelWithLive(pcNode);
	if (pcDelNode)
	{
		pcDelNode->Release();
	}
}

void 
CBoList::DeletePtr(PCNode pcNode)
{
	PCNode pcDelNode = DelWithLiveInPtr(pcNode);
	if (pcDelNode)
	{
		pcDelNode->Release();
	}
}

PCNode
CBoList::DelWithLive(PCNode pcNode)
{
	PCNode pcDelNode;

	if (NULL == pcNode || 
		NULL == (pcDelNode = Find(pcNode)))
	{
		return (NULL);
	}

	PCNode pcPrev = pcDelNode->GetPrev();
	PCNode pcNext = pcDelNode->GetNext();

	if (NULL != pcPrev && 
		NULL != pcNext)
	{
		pcNext->SetPrev(pcPrev);
		pcPrev->SetNext(pcNext);
	}
	else 
	if (NULL != pcPrev && 
		NULL == pcNext)
	{
		m_pcEnd = pcPrev;
		pcPrev->SetNext(NULL);
	}
	else 
	if (NULL == pcPrev && 
		NULL != pcNext)
	{
		m_pcBegin = pcNext;
		pcNext->SetPrev(NULL);
	}
	else
	{
		m_pcBegin = NULL;
		m_pcEnd = NULL;
	}

	return (pcDelNode);
}

PCNode 
CBoList::DelWithLiveInPtr(PCNode pcDelNodePtr)
{
	if (NULL == pcDelNodePtr)
	{
		return (NULL);
	}

	PCNode pcPrev = pcDelNodePtr->GetPrev();
	PCNode pcNext = pcDelNodePtr->GetNext();

	if (NULL != pcPrev && 
		NULL != pcNext)
	{
		pcNext->SetPrev(pcPrev);
		pcPrev->SetNext(pcNext);
	}
	else 
	if (NULL != pcPrev && 
		NULL == pcNext)
	{
		m_pcEnd = pcPrev;
		pcPrev->SetNext(NULL);
	}
	else 
	if (NULL == pcPrev && 
		NULL != pcNext)
	{
		m_pcBegin = pcNext;
		pcNext->SetPrev(NULL);
	}
	else
	{
		m_pcBegin = NULL;
		m_pcEnd = NULL;
	}

	return (pcDelNodePtr);
}

void
CBoList::Reset()
{
	CNode *pcTmp;
	PCNode pcCurrent = GetBegin();
	
	while(pcCurrent)
	{
		pcTmp = pcCurrent->GetNext();
		pcCurrent->Release();

		pcCurrent = pcTmp;
	}

	m_pcBegin = NULL;
	m_pcEnd = NULL;
}

PCNode 
CBoList::Find(const PCNode pcNode)
{
	PCNode pcCurr = GetBegin();

	while (pcCurr)
	{
		if (!pcCurr->CompareTo(pcNode))
		{
			return pcCurr;
		}

		pcCurr = pcCurr->GetNext();
	}

	return (NULL);
}

PCNode 
CBoList::DelBeginWithLive()
{
	PCNode pcDelNode = m_pcBegin;
	PCNode pcNext = pcDelNode->GetNext();

	if (NULL != pcNext)
	{
		m_pcBegin = pcNext;
		pcNext->SetPrev(NULL);
	}
	else
	{
		m_pcBegin = NULL;
		m_pcEnd = NULL;
	}

	return (pcDelNode);
}

PCNode 
CBoList::DelEndWithLive()
{
	PCNode pcDelNode = m_pcEnd;
	PCNode pcPrev = pcDelNode->GetPrev();

	if (NULL != pcPrev)
	{
		m_pcEnd = pcPrev;
		pcPrev->SetNext(NULL);
	}
	else
	{
		m_pcBegin = NULL;
		m_pcEnd = NULL;
	}

	return (pcDelNode);
}

