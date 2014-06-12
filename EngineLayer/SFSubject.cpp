#include "StdAfx.h"
#include "SFSubject.h"
#include "SFObserver.h"

SFSubject::SFSubject(void)
{
}

SFSubject::~SFSubject(void)
{
}

BOOL SFSubject::AddObserver( SFObserver* pObject )
{
	if(TRUE == CheckDuplicate(pObject))
		return FALSE;

	m_Observerlist.push_back(pObject);

	return TRUE;
}

BOOL SFSubject::CheckDuplicate( SFObserver* pObject )
{
	BOOL bDuplicate = FALSE;
	Observerlist::iterator iter = m_Observerlist.begin();

	for(; iter != m_Observerlist.end(); iter++)
	{
		if(*iter == pObject)
		{
			bDuplicate = TRUE;
			break;
		}
	}

	return bDuplicate;
}

BOOL SFSubject::DelObserver( SFObserver* pObject )
{
	Observerlist::iterator iter = m_Observerlist.begin();

	for(; iter != m_Observerlist.end(); iter++)
	{
		if(*iter == pObject)
		{
			m_Observerlist.erase(iter);
			break;
		}
	}

	return TRUE;
}

BOOL SFSubject::PropagateMessage( SFMessage* pMessage )
{
	Observerlist::iterator iter = m_Observerlist.begin();

	for(; iter != m_Observerlist.end(); iter++)
	{
		SFObserver* pObserver = *iter;
		pObserver->OnMessage(this, pMessage);
	}

	return TRUE;
}