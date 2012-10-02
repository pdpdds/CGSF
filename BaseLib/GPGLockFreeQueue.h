#pragma once

//
// Game Programming Gems 6
// Lock free multithreaded algorithms
// By Toby Jones (thjones@microsoft.com)
// Supports Microsoft Visual C++ and GCC.
//

#include <iostream>
#include <new>
#include <cassert>
#include <vector>
#include <algorithm>

#include <windows.h>
#include <process.h>

#ifdef _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4311) // pointer truncation
#endif


namespace GPG
{


	//
	// uint32_t is defined in standard C, but not yet in standard C++.
	//
	typedef unsigned int uint32_t;

	//------------------------------------------------------------------------------
	//
	// Definitions of CAS.
	//
	//------------------------------------------------------------------------------

	//
	// Define what code we will be using for CAS.
	// Intrinsic only available on Visual C++.
	//
#ifndef CAS
#define CAS CAS_assembly
	//#define CAS CAS_intrinsic
	//#define CAS CAS_windows
#endif

	// this function is atomic
	// bool CAS(uint32_t *ptr, uint32_t oldVal, uint32_t newVal) {
	//     if(*ptr == oldVal) {
	//         *ptr = newVal;
	//         return true;
	//     }
	//     return false;
	// }

	//
	// All of the CAS functions will operate on a node.  So we define node first.
	//
	template<typename T> struct node {
		T value;
		node<T> * volatile pNext;

		node() : value(), pNext(NULL) {}
		node(T v) : pNext(NULL), value(v) {}
	};

	// CAS will assume a multi-processor machine (versus multithread on a single processor).
	// On a single processor machine, it might not make sense to spin on a CAS because
	// if it fails, there is no way to succeed until the another thread runs (which will be
	// on the same processor).  In these cases, if CAS is required to succeed, it might make
	// more sense to yield the processor to the other thread via Sleep(1).

	// Since we are assuming a multi-processor machine, we will need to use 'lock cmpxchg'
	// instead of just cmpxchg, as the bus needs to be locked to synchronize the processors
	// access to memory.

	//
	// Define a version of CAS which uses x86 assembly primitives.
	//
	template<typename T>
	bool CAS_assembly(node<T> * volatile * _ptr, node<T> * oldVal, node<T> * newVal)
	{
		register bool f;

#ifdef __GNUC__
		__asm__ __volatile__(
			"lock; cmpxchgl %%ebx, %1;"
			"setz %0;"
			: "=r"(f), "=m"(*(_ptr))
			: "a"(oldVal), "b" (newVal)
			: "memory");
#else
		_asm
		{
			mov ecx,_ptr
				mov eax,oldVal
				mov ebx,newVal
				lock cmpxchg [ecx],ebx
				setz f
		}
#endif // __GNUC__

		return f;
	}

	//
	// Define a version of CAS which uses the Visual C++ InterlockedCompareExchange intrinsic.
	//
#ifdef _MSC_VER

	// Define intrinsic for InterlockedCompareExchange
	extern "C" long __cdecl _InterlockedCompareExchange(long volatile * Dest, long Exchange, long Comp);

#pragma intrinsic (_InterlockedCompareExchange)

	template<typename T>
	bool CAS_intrinsic(node<T> * volatile * _ptr, node<T> * oldVal, node<T> * newVal)
	{
		return _InterlockedCompareExchange((long *)_ptr, (long)newVal, (long)oldVal) == (long)oldVal;
	}

#endif  // _MSC_VER

	//
	// Define a version of CAS which uses the Windows API InterlockedCompareExchange.
	//
	template<typename T>
	bool CAS_windows(node<T> * volatile * _ptr, node<T> * oldVal, node<T> * newVal)
	{
		return InterlockedCompareExchange((long *)_ptr, (long)newVal, (long)oldVal) == (long)oldVal;
	}

	//------------------------------------------------------------------------------
	//
	// Definitions of CAS2.
	//
	//------------------------------------------------------------------------------

	//
	// Define what code we will be using for CAS.
	// Intrinsic only available on Visual C++.
	// Windows version only available on Windows Vista.
	//
#ifndef CAS2
#define CAS2 CAS2_assembly
	//#define CAS2 CAS2_intrinsic
	//#define CAS2 CAS2_windows
#endif

	//
	// Define a version of CAS2 which uses x86 assembly primitives.
	//
	template<typename T>
	bool CAS2_assembly(node<T> * volatile * _ptr, node<T> * old1, uint32_t old2, node<T> * new1, uint32_t new2)
	{
		register bool f;
#ifdef __GNUC__
		__asm__ __volatile__(
			"lock; cmpxchg8b %1;"
			"setz %0;"
			: "=r"(f), "=m"(*(_ptr))
			: "a"(old1), "b" (new1), "c" (new2), "d" (old2)
			: "memory");
#else
		_asm
		{
			mov esi,_ptr
				mov eax,old1
				mov edx,old2
				mov ebx,new1
				mov ecx,new2
				lock cmpxchg8b [esi]
			setz f
		}
#endif
		return f;
	}

	//
	// Define a version of CAS2 which uses the Visual C++ InterlockedCompareExchange64 intrinsic.
	//
#ifdef _MSC_VER

	// Define intrinsic for InterlockedCompareExchange64
	extern "C" __int64 __cdecl _InterlockedCompareExchange64(__int64 volatile * Destination, __int64 Exchange, __int64 Comperand);

#pragma intrinsic (_InterlockedCompareExchange64)

	template<typename T>
	bool CAS2_intrinsic(node<T> * volatile * _ptr, node<T> * old1, uint32_t old2, node<T> * new1, uint32_t new2)
	{
		LONGLONG Comperand = reinterpret_cast<LONG>(old1) | (static_cast<LONGLONG>(old2) << 32);
		LONGLONG Exchange  = reinterpret_cast<LONG>(new1) | (static_cast<LONGLONG>(new2) << 32);

		return _InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(_ptr), Exchange, Comperand) == Comperand;
	}

#endif  // _MSC_VER

	//
	// Define a version of CAS2 which uses the Windows API InterlockedCompareExchange64.
	// InterlockedCompareExchange64 requires Windows Vista
	//
#if WINVER >= 0x0600

	// LONGLONG InterlockedCompareExchange64(LONGLONG volatile * Destination, LONGLONG Exchange, LONGLONG Comperand);

	template<typename T>
	bool CAS2_windows(node<T> * volatile * _ptr, node<T> * old1, uint32_t old2, node<T> * new1, uint32_t new2)
	{
		LONGLONG Comperand = reinterpret_cast<long>(old1) | (static_cast<LONGLONG>(old2) << 32);
		LONGLONG Exchange  = reinterpret_cast<long>(new1) | (static_cast<LONGLONG>(new2) << 32);

		return InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile *>(_ptr), Exchange, Comperand) == Comperand;
	}
#endif  // WINVER >= 0x0600

	//------------------------------------------------------------------------------
	//
	// Parameterized Lock-free Stack
	//
	//------------------------------------------------------------------------------
	template<typename T> class LockFreeStack {
		// NOTE: the order of these members is assumed by CAS2.
		node<T> * volatile _pHead;
		volatile uint32_t  _cPops;

	public:
		void Push(node<T> * pNode);
		node<T> * Pop();

		LockFreeStack() : _pHead(NULL), _cPops(0) {}
	};

	template<typename T> void LockFreeStack<T>::Push(node<T> * pNode)
	{
		while(true)
		{
			pNode->pNext = _pHead;
			if(CAS(&_pHead, pNode->pNext, pNode))
			{
				break;
			}
		}
	}

	template<typename T> node<T> * LockFreeStack<T>::Pop()
	{
		while(true)
		{
			node<T> * pHead = _pHead;
			uint32_t  cPops = _cPops;
			if(NULL == pHead)
			{
				return NULL;
			}

			// NOTE: Memory reclaimation is difficult in this context.  If another thread breaks in here
			// and pops the head, and then frees it, then pHead->pNext is an invalid operation.  One solution
			// would be to use hazard pointers (http://researchweb.watson.ibm.com/people/m/michael/ieeetpds-2004.pdf).

			node<T> * pNext = pHead->pNext;
			if(CAS2(&_pHead, pHead, cPops, pNext, cPops + 1))
			{
				return pHead;
			}
		}
	}

	//------------------------------------------------------------------------------
	//
	// Parameterized Lock-free Queue
	//
	//------------------------------------------------------------------------------
	template<typename T> class LockFreeQueue {
		// NOTE: the order of these members is assumed by CAS2.
		node<T> * volatile _pHead;
		volatile uint32_t  _cPops;
		node<T> * volatile _pTail;
		volatile uint32_t  _cPushes;

	public:
		void Add(node<T> * pNode);
		node<T> * Remove();

		LockFreeQueue(node<T> * pDummy) : _cPops(0), _cPushes(0)
		{
			_pHead = _pTail = pDummy;
		}
	};

	template<typename T> void LockFreeQueue<T>::Add(node<T> * pNode) {
		pNode->pNext = NULL;

		uint32_t cPushes;
		node<T> * pTail;

		while(true)
		{
			cPushes = _cPushes;
			pTail = _pTail;

			// NOTE: The Queue has the same consideration as the Stack.  If _pTail is
			// freed on a different thread, then this code can cause an access violation.

			// If the node that the tail points to is the last node
			// then update the last node to point at the new node.
			if(CAS(&(_pTail->pNext), reinterpret_cast<node<T> *>(NULL), pNode))
			{
				break;
			}
			else
			{
				// Since the tail does not point at the last node,
				// need to keep updating the tail until it does.
				CAS2(&_pTail, pTail, cPushes, _pTail->pNext, cPushes + 1);
			}
		}

		// If the tail points to what we thought was the last node
		// then update the tail to point to the new node.
		CAS2(&_pTail, pTail, cPushes, pNode, cPushes + 1);
	}

	template<typename T> node<T> * LockFreeQueue<T>::Remove() {
		T value = T();
		node<T> * pHead;

		while(true)
		{
			uint32_t cPops = _cPops;
			uint32_t cPushes = _cPushes;
			pHead = _pHead;
			node<T> * pNext = pHead->pNext;

			// Verify that we did not get the pointers in the middle
			// of another update.
			if(cPops != _cPops)
			{
				continue;
			}
			// Check if the queue is empty.
			if(pHead == _pTail)
			{
				if(NULL == pNext)
				{
					pHead = NULL; // queue is empty
					break;
				}
				// Special case if the queue has nodes but the tail
				// is just behind. Move the tail off of the head.
				CAS2(&_pTail, pHead, cPushes, pNext, cPushes + 1);
			}
			else if(NULL != pNext)
			{
				value = pNext->value;
				// Move the head pointer, effectively removing the node
				if(CAS2(&_pHead, pHead, cPops, pNext, cPops + 1))
				{
					break;
				}
			}
		}
		if(NULL != pHead)
		{
			pHead->value = value;
		}
		return pHead;
	}

	//------------------------------------------------------------------------------
	//
	// Parameterized Lock-free Freelist
	//
	//------------------------------------------------------------------------------
	template<typename T> class LockFreeFreeList {
		//
		// Memory reclaimation is generally difficult with lock-free algorithms,
		// so we bypass the situation by making the object own all of the memory,
		// and creating and destroying the nodes on the thread that controls the
		// object's lifetime.  Any thread synchronization should be done at that
		// point.
		//
		LockFreeStack<T> _Freelist;
		node<T> * _pObjects;
		const uint32_t _cObjects;

	public:
		//
		// cObjects is passed to the constructor instead of a template parameter
		// to minimize the code bloat of multiple freelists with varying sizes,
		// but each using the same underlying type.
		//
		LockFreeFreeList(uint32_t cObjects) : _cObjects(cObjects)
		{
			//
			// The Freelist may live on the stack, so we allocate the
			// actual nodes on the heap to minimize the space hit.
			//
			_pObjects = new node<T>[cObjects];
			FreeAll();
		}
		~LockFreeFreeList()
		{
#ifdef _DEBUG
			for(uint32_t ix = 0; ix < _cObjects; ++ix)
			{
				assert(_Freelist.Pop() != NULL);
			}
#endif

			delete[] _pObjects;
		}
		void FreeAll()
		{
			for(uint32_t ix = 0; ix < _cObjects; ++ix)
			{
				_Freelist.Push(&_pObjects[ix]);
			}
		}
		T * NewInstance()
		{
			node<T> * pInstance = _Freelist.Pop();
			return new(&pInstance->value) T;
		}
		void FreeInstance(T * pInstance)
		{
			pInstance->~T();
			_Freelist.Push(reinterpret_cast<node<T> *>(pInstance));
		}
	};
};