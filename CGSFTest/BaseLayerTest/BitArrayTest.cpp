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
	
	//4578번째 비트를 세팅할 것. 4578/8번째 바이트의 4578%8 비트에 비트값이 1로 세팅 
	a.SetAt(4578);

	//328번째 비트를 세팅할 것. 328/8번째 바이트의 328%8 비트에 비트값이 1로 세팅 
	a.SetAt(323);

	// 1로 세팅된 비트의 수 현재 2개가 세팅되어 있으므로 2를 리턴한다. 
	SFASSERT( 2 == a.GetCount());
	
	// 323번째 비트를 XOR한다. 323번째 비트는 1로 세팅되어 있었으므로 연산결과 323번째 비트는 0의 값을 갖게 된다. 
	a.XOrAt(323);
	
	SFASSERT( 1 == a.GetCount());

	SFBitArray b;
	
	char buffer[4] = {1,1,1,1};
	b.Attach((BYTE*)buffer, 4);

	//미리 비트세팅된 버퍼를 BitArray의 초기값으로 사용한다. 
	//4바이트 값 비트 Array 00000001000000010000000100000001 이므로 비트세팅된 개수는 총 4개 
	SFASSERT( 4 == b.GetCount());

	b.Detach();

	return true;
}

bool BitArrayTest::Finally()
{
	return true;
}