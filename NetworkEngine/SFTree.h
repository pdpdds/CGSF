#pragma once
#include "ace/RB_Tree.h"

template<class EXT_ID, class INT_ID>
class SFTree : public ACE_RB_Tree<EXT_ID, INT_ID, ACE_Less_Than<EXT_ID>, ACE_Null_Mutex>
{
public:
	SFTree(void){}
	virtual ~SFTree(void){}

	BOOL Add(EXT_ID T, INT_ID Q)
	{
		int Result = bind(T, Q);

		if(Result != 0)
		{
			SFASSERT(0);
			return FALSE;
		}

		return TRUE;
	}

	INT_ID Find(EXT_ID T)
	{
		INT_ID Q;
		find(T, Q);

		return Q;
	}

	INT_ID Remove(EXT_ID T)
	{
		INT_ID Q;
		int Result = unbind(T, Q);

		if(Result != 0)
		{
			SFASSERT(0);
			return NULL;
		}

		return Q;
	}

protected:

private:
};

