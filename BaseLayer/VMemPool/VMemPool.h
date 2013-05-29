// VMemPool.h: interface for the CVMemMan class.
//
// Virtual Memory Pool Management Class
// by bro (bro@shinbiro.com)
// (Cho,Kyung-min)
// 2002-02-19
//
//--------------------------------------------------------------------
// History.
// 2002-02-19 : Fisrt Code.
// 2002-02-20 : vmXXX public Function supported 
//              DEBUG_NEW supported ( but not impl yet.. )
// 2002-03-08 : New mechanism : alloc bitset, not use std::list is so heavy.
//              just i use 'Free-MemoryBlock-Queue'
//              Support Helper Function: IsBadPtr.
//--------------------------------------------------------------------
// Manual
// Usage
/*

class CObj : public CVMemPool<>
{
public:
   :
};

CObj* pObj = new CObj; // Virtual Memory Pool 1000 blocks created, allocated 1st block
CObj* pObj2 = new CObj;// allocated second block
delete pObj;           // first block deleted
delete pObj2;          // second block deleted

*//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VMEMPOOL_H__BD52675B_6C82_4CDE_8618_0141D7A4653F__INCLUDED_)
#define AFX_VMEMPOOL_H__BD52675B_6C82_4CDE_8618_0141D7A4653F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BitSet.h"
#include "Macro.h"

struct CSLock
	{
		CRITICAL_SECTION  cs;
		CSLock(){ InitializeCriticalSection(&cs); }
		~CSLock(){ DeleteCriticalSection(&cs); }
		void Lock(){ EnterCriticalSection(&cs); }
		void Unlock() { LeaveCriticalSection(&cs); }
	};

//====================================================================
// CVMemPool<int T> : Virtual Memory Pool Management Class
//--------------------------------------------------------------------
// Virtual Memory Pool for managing same object block
//====================================================================
template <class objT,DWORD _dwPoolSizeT = 1000>
class CVMemPool  
{
public:
	//===========================================================
	// Name   : vmInitPool
	// Desc   : Initilize Pool ( you must call this before 'new',
	//          if you want to reconfigure Pool. if you don't call
	//          it, then Pool works with default configuration )
	// Param  : nPoolSize - Size of Pool (Object Block Total count)
	//          bAllocNow - Virtual Allocation right now
	//          nObjSize - bAllocNow is true, then you must give Object size 
	//                     cos, i don't know object's size in this time )
	// Return : if bAllocNow is true, so pool try to alloc, but it fail
	//          then it return false.
	//-----------------------------------------------------------
	// Coder  Dated                    Desc
	// bro    2002-02-20
	//===========================================================
	static inline BOOL vmInitPool( DWORD dwPoolSize , BOOL bAllocNow = FALSE, DWORD dwObjSize = 0 )
	{
		ms_dwPoolSize = dwPoolSize;
		if( bAllocNow && dwObjSize > 0 )
		{
			ms_dwObjSize = dwObjSize;
			if( !AllocPool(ms_dwPoolSize,ms_dwObjSize) )
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	
	//===========================================================
	// Name   : vmCleanUp
	// Desc   : you should put it on term code
	//          but, it's optional. cos all freed safely when 
	//          process terminate.
	// Param  : 
	// Return : 
	//-----------------------------------------------------------
	// Coder  Dated                    Desc
	// bro    2002-02-20
	//===========================================================
	static inline void vmCleanUp()
	{
		if( ms_pMemPool )
			VirtualFree( ms_pMemPool, DWORD(ms_dwPoolSize*ms_dwObjSize), MEM_RELEASE|MEM_DECOMMIT );
		DeleteCriticalSection( &ms_csLock );
	}

	//===========================================================
	// Name   : vmGetPoolInfo
	// Desc   : Get Pool information
	//          If you interest of pool memory usage statistics
	//          then you can use it for that.
	// Param  : pdwPoolSize - [out] Pool Object total count( Not bytes )
	//          pdwAllocObjCount - [out] Current allocated object count
	//          pdwObjSize - [out] one object bytes size.
	// Return : 
	//-----------------------------------------------------------
	// Coder  Dated                    Desc
	// bro    2002-02-20
	//===========================================================
	static inline BOOL vmGetPoolInfo( DWORD* pdwPoolSize, DWORD* pdwAllocObjCount, DWORD* pdwObjSize )
	{
		BOOL bRet = TRUE;
		if( !ms_pMemPool )
			bRet = FALSE;
		if( pdwPoolSize ) *pdwPoolSize = ms_dwPoolSize;
		if( pdwAllocObjCount ) *pdwAllocObjCount = ms_dwAllocObjCount;
		if( pdwObjSize ) *pdwObjSize = ms_dwObjSize;

		return bRet;
	}
	// memory block validation checker
	static inline BOOL vmIsBadPtr( void* p )
	{
		// is follow basic rule of pool block?
		if( p && p>=ms_pData && p<=ms_pData+(ms_dwPoolSize*ms_dwObjSize)&&
				((char*)p-ms_pData)%ms_dwObjSize == 0 )
		{
			// is really allocated in pool ?
			DWORD id = Addr2Id(p);
			return (ms_bsAllocSet.Get(id)==0);
		}
		return TRUE;
	}
	
private:
	static CBitSet			ms_bsAllocSet;			// bitset for allocated blocks
	static DWORD			ms_dwSrtFree;			// freeblock start marker in circular queue
	static DWORD			ms_dwEndFree;			// freeblock end marker
	static BOOL				ms_bEmptyFree;			// state value for empry queue
	static BOOL				ms_bFullFree;			// state value for full queue
	
	static DWORD			ms_dwFreeQueueSize;
	static DWORD			ms_dwTotalSize;


	static CSLock	ms_csLock;
	static char*	ms_pMemPool;					// Virtual memory pool pointer
	static char*	ms_pData;						// real data pointer
	static DWORD	ms_dwPoolSize;					// Total Pool object counter
	static DWORD	ms_dwObjSize;					// one object size (bytes)
	static DWORD	ms_dwAllocObjCount;				// current allocated counts
	
	

	// translate address to block numer
	static inline DWORD	Addr2Id( void* p )
	{
		return DWORD((char*)p-ms_pData)/ms_dwObjSize;
	}
	// translate block number to address
	static inline void* Id2Addr( DWORD id )
	{
		return ms_pData+(ms_dwObjSize*id);
	}

	// add free block number to free-circular queue
	static inline BOOL InsertFree( DWORD id )
	{
		if( (ms_dwEndFree+1) % ms_dwFreeQueueSize == ms_dwSrtFree )
		{
			// OverFlow
			return FALSE;
		}
		// write newbie free block
		*((DWORD*)ms_pMemPool+ms_dwEndFree) = id;
		ms_dwEndFree = ++ms_dwEndFree % ms_dwFreeQueueSize;
		// free block bit set 
		ms_bsAllocSet.Set( id, 0 );

		return TRUE;
	}
	// pop free block from free-circular queue
	static inline DWORD PopFree()
	{
		if( ms_dwSrtFree == ms_dwEndFree )
		{
			// Underflow ( no block for use )
			return 0xFFFFFFFF;
		}

		// get free id
		DWORD id = *((DWORD*)ms_pMemPool+ms_dwSrtFree);
		// bit set it is used.
		ms_bsAllocSet.Set( id, 1 );
		
		// increment start free marker
		ms_dwSrtFree=(++ms_dwSrtFree)%ms_dwFreeQueueSize;
		return id;
	}

	static inline BOOL AllocPool(DWORD dwPoolSize, DWORD dwObjSize)
	{
		DWORD dwUsedBytes = ms_bsAllocSet.sCalcUsedBytes(dwPoolSize);
		DWORD dwFreeQueue = (dwPoolSize+1)*sizeof(DWORD);
		DWORD dwTotalSize = dwUsedBytes + dwFreeQueue + dwPoolSize*dwObjSize;

		ms_pMemPool = (char*)VirtualAlloc( NULL, dwTotalSize, 
							MEM_RESERVE|MEM_COMMIT|MEM_TOP_DOWN, 
							PAGE_READWRITE );
		if( !ms_pMemPool)
			return FALSE;
		ms_dwTotalSize = dwTotalSize;
		ms_dwFreeQueueSize = dwPoolSize+1;
		ms_pData = ms_pMemPool+dwUsedBytes+dwFreeQueue;

		// allocation bitset create
		if(!ms_bsAllocSet.Create( dwPoolSize, TRUE, TRUE, ms_pMemPool+dwFreeQueue,dwUsedBytes ) )
			return FALSE;
		
		// free circular queue init
		ms_dwSrtFree = 0;
		ms_dwEndFree = 0;
		ms_bEmptyFree = TRUE;
		ms_bFullFree = FALSE;
		for( DWORD i = 0 ; i < dwPoolSize; i++)
		{
			// all block is available when pool is created.
			InsertFree(i);
		}

		return TRUE;
	}
public:
	// DEBUG_NEW support function ( No support memory tracking for MFC )
	inline void* operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
	{
		return operator new( nSize );
	}
	inline void operator delete(void* p, LPCSTR lpszFileName, int nLine)
	{
		operator delete( p );
	}

	// new operator override
	void* operator new( size_t size )
	{
		ASSERT( size > 0 );
		// pointer for newed free address in pool
		void* pNew = NULL;
		
		ms_csLock.Lock();


		//====================================================
		// if pool isn't init, then init
		if( !ms_pMemPool )
		{
			if( !ms_dwPoolSize ) ms_dwPoolSize = _dwPoolSizeT;
			ms_dwObjSize = size;
			if( !AllocPool(ms_dwPoolSize,ms_dwObjSize) )
			{
				ms_csLock.Unlock();
				return NULL;
			}
		}
		
		// get available block in pool
		DWORD id = PopFree();
		if( id == 0xFFFFFFFF )
		{
			return NULL;
		}
		pNew = Id2Addr(id);
		
		//====================================================
		if( pNew )
			ms_dwAllocObjCount++;
		else
			;//allocation fail!
		ms_csLock.Unlock();
		return pNew;
	}
	// free memory
	void operator delete( void* p )
	{
		ASSERT( p && 
				p>=ms_pData&&
				p<=ms_pData+(ms_dwPoolSize*ms_dwObjSize)&&
				((char*)p-ms_pData)%ms_dwObjSize == 0 );
				

		if( !p ) return;
		
		// lock pool
		ms_csLock.Lock();

		//====================================================
		// add to 'free-queue'for reusing this block 
		InsertFree( Addr2Id(p) );

		ms_dwAllocObjCount--;
		//====================================================
		
		// unlock
		ms_csLock.Unlock();
	}
};

// declaration static valuables
template <typename T,DWORD F> CBitSet CVMemPool<T,F>::ms_bsAllocSet;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwSrtFree;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwEndFree;
template <typename T,DWORD F> BOOL CVMemPool<T,F>::ms_bEmptyFree;
template <typename T,DWORD F> BOOL CVMemPool<T,F>::ms_bFullFree;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwTotalSize;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwFreeQueueSize;
template <typename T,DWORD F> char* CVMemPool<T,F>::ms_pData;

template <typename T,DWORD F> CSLock CVMemPool<T,F>::ms_csLock;
template <typename T,DWORD F> char*	CVMemPool<T,F>::ms_pMemPool = NULL;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwObjSize;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwAllocObjCount;
template <typename T,DWORD F> DWORD CVMemPool<T,F>::ms_dwPoolSize;


#endif // !defined(AFX_VMEMPool_H__BD52675B_6C82_4CDE_8618_0141D7A4653F__INCLUDED_)
