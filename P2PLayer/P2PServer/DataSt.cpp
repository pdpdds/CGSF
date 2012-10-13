////////////////////////////////////////////////////////////////////////////////
//
// 작성자 : 이용휴
// 작성일 : 2003.01.28
// 설  명 :
//

/////////////////////////////////////////////////////////////////////////////////////////
//
#include "AllOcfCliSys.h"
#include "AllOcfCli.h"

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

/////////////////////////////////////////////////////////////////////////////////////////
// 
CBtree::CBtree() 
{
	m_pcRoot = NULL;
	m_pcFirst = NULL;
	m_pcLast = NULL;
}

CBtree::~CBtree()
{
	PCTreeNode pTmp;
	PCTreeNode pCurrent = GetFirst();

	while(pCurrent)
	{
		pTmp = (PCTreeNode)(pCurrent->GetNext());
		pCurrent->Release();

		pCurrent = pTmp;
	}
}

PCTreeNode 
CBtree::GetLeftEnd(PCTreeNode pNode)
{
	PCTreeNode pNodeLeft = pNode;

	while (1)
	{
		if (NULL == pNodeLeft->GetLeft())
		{
			break;
		}

		pNodeLeft = pNodeLeft->GetLeft();
	}

	return pNodeLeft;
}

PCTreeNode 
CBtree::GetRightEnd(PCTreeNode pNode)
{
	PCTreeNode pNodeRight = pNode;

	while (1)
	{
		if (NULL == pNodeRight->GetRight())
		{
			break;
		}

		pNodeRight = pNodeRight->GetRight();
	}

	return pNodeRight;
}

BOOL
CBtree::Add(PCTreeNode pcTreeNode)
{
	pcTreeNode->Reset();

	if (m_pcRoot == NULL)
	{
		m_pcRoot = pcTreeNode;
		m_pcFirst = pcTreeNode;
		m_pcLast = pcTreeNode;
		
		return TRUE;
	}

	BOOL bRet = FALSE;
	PCTreeNode pcExp = m_pcRoot;

	while(1)
	{
		LONG lResult = pcExp->CompareTo(pcTreeNode);

		if (0 == lResult)
		{
			break;
		}
		else if (lResult > 0)
		{
			if (pcExp->GetRight() == NULL)
			{
				pcTreeNode->SetParent(pcExp);
				pcExp->SetRight(pcTreeNode);

				pcTreeNode->SetPrev(pcExp);
				pcTreeNode->SetNext(pcExp->GetNext());
				if (pcExp->GetNext())
				{
					pcExp->GetNext()->SetPrev(pcTreeNode);
				}
				pcExp->SetNext(pcTreeNode);
				
				bRet = TRUE;
				break;
			}

			pcExp = pcExp->GetRight();
		}
		else if (lResult < 0)
		{
			if (pcExp->GetLeft() == NULL)
			{
				pcTreeNode->SetParent(pcExp);
				pcExp->SetLeft(pcTreeNode);

				pcTreeNode->SetPrev(pcExp->GetPrev());
				pcTreeNode->SetNext(pcExp);
				if (pcExp->GetPrev())
				{
					pcExp->GetPrev()->SetNext(pcTreeNode);
				}
				pcExp->SetPrev(pcTreeNode);

				bRet = TRUE;
				break;
			}

			pcExp = pcExp->GetLeft();
		}
	}

	if (TRUE == bRet && NULL == pcTreeNode->GetPrev())
	{
		m_pcFirst = pcTreeNode;
	}

	if (TRUE == bRet && NULL == pcTreeNode->GetNext())
	{
		m_pcLast = pcTreeNode;
	}

	return (bRet);
}

PCTreeNode
CBtree::Find(const PCTreeNode pcTreeNode)
{
	if (NULL == m_pcRoot)
	{
		return NULL;
	}

	PCTreeNode pcExp = m_pcRoot;
	while(1)
	{
		LONG lResult = pcExp->CompareTo(pcTreeNode);

		if (lResult == 0)
		{
			return pcExp;
		}
		else if (lResult > 0)
		{
			if (pcExp->GetRight() == NULL)
			{
				return NULL;
			}

			pcExp = pcExp->GetRight();
		}
		else if (lResult < 0)
		{
			if (pcExp->GetLeft() == NULL)
			{
				return NULL;
			}

			pcExp = pcExp->GetLeft();
		}
	}
}

BOOL 
CBtree::Delete(PCTreeNode pcTreeNode)
{
	PCTreeNode pcDelNode = DelWithLive(pcTreeNode);
	if (NULL != pcDelNode)
	{
		pcDelNode->Release();
		return TRUE;
	}

	return FALSE;
}

void 
CBtree::DelNodeAndLink(PCTreeNode pcTreeNode)
{
	if (pcTreeNode->GetNext())
	{
		pcTreeNode->GetNext()->SetPrev(pcTreeNode->GetPrev());
	}

	if (pcTreeNode->GetPrev())
	{
		pcTreeNode->GetPrev()->SetNext(pcTreeNode->GetNext());
	}

	if (m_pcFirst == pcTreeNode)
	{
		m_pcFirst = (PCTreeNode)(pcTreeNode->GetNext());
	}

	if (m_pcLast == pcTreeNode)
	{
		m_pcLast = (PCTreeNode)(pcTreeNode->GetPrev());
	}
}

PCTreeNode
CBtree::DelWithLive(PCTreeNode pcTreeNode)
{
	PCTreeNode pcDelNode = Find(pcTreeNode);
	if (NULL == pcDelNode)
	{
		return NULL;
	}

	PCTreeNode pcRepNode = NULL;
	PCTreeNode pcDelNodeParent = pcDelNode->GetParent();
	PCTreeNode pcDelNodeLeft = pcDelNode->GetLeft();
	PCTreeNode pcDelNodeRight = pcDelNode->GetRight();

	if (NULL == pcDelNodeParent && NULL == pcDelNodeLeft && NULL == pcDelNodeRight)
	{
		m_pcRoot = NULL;
		m_pcFirst = NULL;
		m_pcLast = NULL; 
		
		return (pcDelNode);
	}

 	if (NULL != pcDelNodeLeft && NULL == pcDelNodeRight)
	{
		pcDelNodeLeft->SetParent(pcDelNodeParent);
		DelNodeAndLink(pcDelNode);
		pcRepNode = pcDelNodeLeft;
	}
	else 
	if (NULL == pcDelNodeLeft && NULL != pcDelNodeRight)
	{
		pcDelNodeRight->SetParent(pcDelNodeParent);
		DelNodeAndLink(pcDelNode);
		pcRepNode = pcDelNodeRight;
	}
	else 
	if (NULL != pcDelNodeLeft && NULL != pcDelNodeRight)
	{
		PCTreeNode pcLeftEnd = GetLeftEnd(pcDelNodeRight);
		pcDelNodeLeft->SetParent(pcLeftEnd);
		DelNodeAndLink(pcDelNode);

		if (pcDelNodeRight == pcLeftEnd)
		{
			pcLeftEnd->SetParent(pcDelNodeParent);
			pcLeftEnd->SetLeft(pcDelNodeLeft);
		}
		else
		{
			pcDelNodeRight->SetParent(pcLeftEnd);
			PCTreeNode pcParentLeft = pcLeftEnd->GetParent();
			PCTreeNode pcChildRight = pcLeftEnd->GetRight();

			if (pcChildRight)
			{
				pcChildRight->SetParent(pcParentLeft);
			}

			pcLeftEnd->SetLeft(pcDelNodeLeft);
			pcLeftEnd->SetRight(pcDelNodeRight);
			pcLeftEnd->SetParent(pcDelNodeParent);
			pcParentLeft->SetLeft(pcChildRight);
		}

		pcRepNode = pcLeftEnd;
	}
	else
	{
		DelNodeAndLink(pcDelNode);
	}

	if (pcDelNodeParent)
	{
		if (pcDelNodeParent->GetLeft() == pcDelNode)
		{
			pcDelNodeParent->SetLeft(pcRepNode);
		}
		else 
		if (pcDelNodeParent->GetRight() == pcDelNode)
		{
			pcDelNodeParent->SetRight(pcRepNode);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		m_pcRoot = pcRepNode;
	}

	return pcDelNode;
}

void
CBtree::Reset()
{
	PCTreeNode pTmp;
	PCTreeNode pCurrent = GetFirst();

	while(pCurrent)
	{
		pTmp = static_cast<PCTreeNode>(pCurrent->GetNext());
		pCurrent->Release();
		
		pCurrent = pTmp;
	}
	
	m_pcRoot = NULL;
	m_pcFirst = NULL;
	m_pcLast = NULL;
}

