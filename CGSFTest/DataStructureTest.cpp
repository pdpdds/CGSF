	//////////////////////////////////////////////////////////////////////
//게임 프로그래머를 위한 자료구조와 알고리즘 소스 테스트
//////////////////////////////////////////////////////////////////////////	
#include "stdafx.h"
#include "DataStructureTest.h"

#include "Array2D.h"
#include "Queue.h"
#include "Heap.h"

using namespace CGBase;

int CompareIntDescending( int left, int right )
{
    if( left < right )
        return 1;

    if( left > right)
        return -1;

    return 0;
}

DataStructureTest::DataStructureTest(void)
{
}


DataStructureTest::~DataStructureTest(void)
{
}



bool DataStructureTest::Run()
{
/////////////////////////////////
//Array Test
/////////////////////////////////
	Array2D<int> Array2D_( 5, 4 );
	(*Array2D_.Get(4,3)) = 5;

	int* ArrayValue = Array2D_.Get(4,3);

	SFASSERT(*ArrayValue == 5);

/////////////////////////////////
//Queue Test
/////////////////////////////////
	LQueue<int> Queue;
	int Data = 5;
	Queue.Enqueue(Data);
	Queue.Enqueue(Data);

/////////////////////////////////
//Heap Test
/////////////////////////////////
	Heap<int> IntHeap( 100, CompareIntDescending );

	Data = 7;
	IntHeap.Enqueue(Data);
	Data = 10;
	IntHeap.Enqueue(Data);
	Data = 8;
	IntHeap.Enqueue(Data);

	int HeapTop = IntHeap.Item();

	SFASSERT(HeapTop == 7);

	return true;
	
}

