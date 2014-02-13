#include "stdafx.h"
#include "BitArrayTest.h"
#include "SFBitArray.h"

BitArrayTest::BitArrayTest(void)
{
}


BitArrayTest::~BitArrayTest(void)
{
}

bool BitArrayTest::Initialize()
{
	return true;
}

bool BitArrayTest::Run()
{
	SFBitArray a;
	a.SetAt(4578);
	a.SetAt(323);
	SFASSERT( 2 == a.GetCount());
	
	a.XOrAt(323);
	
	SFASSERT( 1 == a.GetCount());

	SFBitArray b;
	
	char buffer[4] = {1,1,1,1};
	b.Attach((BYTE*)buffer, 4);

	SFASSERT( 4 == b.GetCount());

	b.Detach();

	return true;
}

bool BitArrayTest::Finally()
{
	return true;
}