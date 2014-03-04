#pragma once
#include <ace/Malloc_T.h>

typedef ACE_Dynamic_Cached_Allocator<ACE_Null_Mutex> ACEAllocator;

template<class T>
class AceMemoryObject
{
	
public:
	static void* operator new(size_t size)
	{
		return AceMemoryPool->malloc(size);
	}

	static void operator delete(void* p)
	{
		AceMemoryPool->free(p);
	}

	static ACEAllocator* AceMemoryPool;
};

template<class T>
ACEAllocator* AceMemoryObject<T>::AceMemoryPool = new ACEAllocator(1000, sizeof(T));