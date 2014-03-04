#include "stdafx.h"
#include "ACETreeTest.h"
#include "SFTree.h"
#include "ace/os_memory.h"

ACETreeTest::ACETreeTest(void)
{
}


ACETreeTest::~ACETreeTest(void)
{
}

bool ACETreeTest::Run()
{
	ACE::init();


	SFTree<int, int*> tree;
	for(int i = 0; i < 100; i++)
	{
		int* d = 0;
		ACE_NEW_NORETURN(d, int);
		*d = i;
		tree.Add(i, d);
	}

	int *q = 0;

	q = tree.Find(49);

	for(int i = 0; i < 100; i++)
	{
		int *h = tree.Remove(i);
	}

	ACE::fini();

	return true;
}
