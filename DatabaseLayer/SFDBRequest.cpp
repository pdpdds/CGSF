#include "StdAfx.h"
#include "SFDBRequest.h"
#include "SFDBWorker.h"

int SFDBRequest::call()
{
	SFASSERT(m_pMessage != NULL);

	m_pOwner->GetDatabase()->Call(m_pMessage);
	m_pOwner->GetDatabase()->Release(m_pMessage);

	m_pMessage = NULL;
	
	return 0;
}

