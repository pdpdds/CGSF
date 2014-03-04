#pragma once
#include <list>
#include "SFObject.h"

class SFObserver;
class SFMessage;

class SFSubject : public SFObject
{
	typedef std::list<SFObserver*> Observerlist;
public:
	SFSubject(void);
	virtual ~SFSubject(void);

	BOOL AddObserver(SFObserver* pObject);
	BOOL DelObserver(SFObserver* pObject);

	BOOL PropagateMessage(SFMessage* pMessage);

	int GetObserverCount(){return m_Observerlist.size();}

protected:
	BOOL CheckDuplicate(SFObserver* pObject);

private:
	Observerlist m_Observerlist;
};
