#include "StdAfx.h"
#include "SFDBRequest.h"
#include "SFDBWorker.h"
#include "SFDatabase.h"

int SFDBRequest::call()
{
	SFASSERT(m_pMessage != NULL);

/////////////////////////////////////////////////
//Call method에서 m_pMessage 객체를 회수한다.
/////////////////////////////////////////////////
	m_pOwner->GetDatabase()->Call(m_pMessage);

	m_pMessage = NULL;
	
	return 0;
}

