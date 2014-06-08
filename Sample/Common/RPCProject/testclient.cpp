#include "stdafx.h"
#include "testclient.h"


implement_serial(TestArg)
implement_serial(TestArgImpl)
implement_serial(TestArg1)

void TestArg1::Serialize(CPstream& ar)
{
	if (ar.isLoading())
		ar >> ll;
	else
		ar << ll;
}