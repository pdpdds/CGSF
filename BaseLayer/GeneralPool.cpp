#include "stdafx.h"
#include "generalpool.h"
#include <assert.h>
#include <stdio.h>

//------------------------------------------------------------------- my codeing style

#define null NULL

// debug 모드에서만 유효성 체크를 실시함
#ifdef _DEBUG 
#define validate_page( p )	valid( p );
#else
#define validate_page( p )	__noop
#endif

#ifdef _DEBUG
#define assert_msg( msg, exp )		assert( (exp) && msg )
#else
#define assert_msg( msg, exp)		__noop
#endif

	struct CriSecT
	{
		CRITICAL_SECTION m_cs;
		CriSecT() { InitializeCriticalSection( &m_cs ); }
		~CriSecT() { DeleteCriticalSection( &m_cs ); }

		__forceinline void lock() { EnterCriticalSection( &m_cs ); }
		__forceinline void unlock() { LeaveCriticalSection( &m_cs ); }
	};

	struct NullMutex
	{
		__forceinline void lock() {}
		__forceinline void unlock(){}
	};

	//------------------------------------------------------------------- Runtime Memory

	void*RtMemPool::alloc(unsigned int n)
	{
		return ::malloc( n );
	}

	void RtMemPool::free(void*& p)
	{
		if ( p == null )
			return;
		::free( p );
		p = null;
	}

	RtMemPool _default_allocator;

    
	//------------------------------------------------------------------- GeneralPool

	class GeneralPool_Page;
	class GeneralPool_Impl;

	class GeneralPool_Node
	{
	public:
		GeneralPool_Page*	_owner;
		unsigned int		_size;
		unsigned int		_realSize;

		GeneralPool_Node*	_availablePrev;
		GeneralPool_Node*	_availableNext;

		GeneralPool_Node*	_prev;
		GeneralPool_Node*	_next;

		bool				_allocated;

		__forceinline void		clear(GeneralPool_Page* owner)
		{
			_owner			= owner;
			_realSize		= 0;
			_size			= 0;

			_availablePrev	= null;
			_availableNext	= null;

			_prev			= null;
			_next			= null;

			_allocated		= false;
		}

		__forceinline BYTE*		getMemory()
		{
			return ((BYTE*) this) + sizeof(GeneralPool_Node);
		}
	};

	__forceinline unsigned int		nodeSize()
	{
		return sizeof(GeneralPool_Node);
	}
	__forceinline GeneralPool_Node* Node(void* _pMemoryBlock)
	{
		return (GeneralPool_Node*) (((unsigned int) _pMemoryBlock) - nodeSize());
	}

	__forceinline const GeneralPool_Node*	Node(const void* _pMemoryBlock)
	{
		return (const GeneralPool_Node*) (((unsigned int) _pMemoryBlock) - nodeSize());
	}

	class GeneralPool_Page
	{
	public:
		GeneralPool_Page*	_prev;
		GeneralPool_Page*	_next;

		GeneralPool_Node*	_available;			// 
		unsigned int		_availableSize;

		BYTE				_mem[default_page_size];

		unsigned int		_miss;

		
		GeneralPool_Node*	_firstNode;
		
							GeneralPool_Page() : _next(null), _available(null), _firstNode((GeneralPool_Node*)_mem), _availableSize(0), _miss(0){}

		void				removeAvailableList( GeneralPool_Node* node)
		{
			// 링크를 수정한다
			if (node->_availablePrev == null)
			{
				assert_msg("잘못된 Root 정보", _available == node);
				_available = node->_availableNext;
			}
			else
			{
				node->_availablePrev->_availableNext = node->_availableNext;
			}

			if (node->_availableNext != null)
			{
				node->_availableNext->_availablePrev = node->_availablePrev;
			}

			node->_availablePrev = null;
			node->_availableNext = null;

			// 얼로케이션 플래그를 설정한다
			node->_allocated = true;
		}
	};

	class GeneralPool_Impl
	{
	public:
		GeneralPool_Page*		_missingPage;
		GeneralPool_Page*		_availablePage;
		CriSecT					_mutex;


			 					GeneralPool_Impl();
								~GeneralPool_Impl();

		/** 
			메모리 할당
		 */
		void*					alloc(unsigned int size);

		/**
			메모리 해제한다.
		 */
		void					free(void* _pBuffer);
		
		/** 
			새 페이지 할당
		 */
		void					allocPage();

		/** 
			할당해줄 노드를 분리한다
		 */	
		void					splitAllocNode( GeneralPool_Page* page, GeneralPool_Node* node, unsigned int size);

		/**	
			앞,뒷쪽 노드와 합친다 (앞쪽 노드만 남는다)
		 */
		void					freeMergePrevAndNext(GeneralPool_Node* node);
		/**
			앞쪽 노드와 합친다 (앞쪽 노드만 남는다)
		 */
		void					freeMergePrev(GeneralPool_Node* node);
		/** 
			앞쪽 노드와 합친다 (앞쪽 노드만 남는다)
		 */
		void					freeMergeNext(GeneralPool_Node* node);		
		/**
			노드 해제 - 아무 노드와도 합쳐지지 않는다
		 */
		void					freeMergeNone(GeneralPool_Node* node);

		/** 
			디버그 검사
		 */
		void					valid(GeneralPool_Page* page);

		void					lock();
		void					unlock();

	};


GeneralPool::GeneralPool()
{	
	m_impl = new GeneralPool_Impl( );
}

GeneralPool::~GeneralPool()
{
	delete m_impl;
}


void* GeneralPool::alloc(unsigned int size)
{
	return m_impl->alloc(size);
}


void GeneralPool::free(void*& p)
{
	if (p == null)	{	return;	}
	m_impl->free(p);
	p = null;
}

unsigned int GeneralPool::size(const void*  p)
{
	const GeneralPool_Node*	node;

	if (p == null)	{	return 0;	}

	node = Node(p);

	return node->_realSize;
}

GeneralPool_Impl::GeneralPool_Impl(): _availablePage(null),_missingPage(null)
{
	allocPage();
}

GeneralPool_Impl::~GeneralPool_Impl()
{
	lock();

	GeneralPool_Page*	page;
	GeneralPool_Page*	pPageNext;

	page = _availablePage;

	// 모든 페이지를 지운다
	while (page != null)
	{
		pPageNext = page->_next;
		delete page;
		page = pPageNext;
	}
	page = _missingPage;

	// 모든 페이지를 지운다
	while (page != null)
	{
		pPageNext = page->_next;
		delete page;
		page = pPageNext;
	}

	unlock();

}

inline
void GeneralPool_Impl::lock()
{
	_mutex.lock();
}

inline
void GeneralPool_Impl::unlock()
{
	_mutex.unlock();
}

void GeneralPool_Impl::allocPage()
{
	GeneralPool_Page*	page;

	lock();

	page					= new GeneralPool_Page;

	page->_prev				= null;
	page->_next				= _availablePage;
	page->_availableSize	= default_page_size;

	if( _availablePage )
		_availablePage->_prev = page;

	_availablePage			= page;

	// 첫번째 노드를 준비한다
	page->_firstNode->clear(page);

	// 첫번째 노드는 할당 가능한 노드
	page->_available = page->_firstNode;

	// 최대 할당 가능한 크기는 헤더를 뺀 페이지 크기(maximum space) = (page size) - (header size)
	page->_firstNode->_size = page->_availableSize - nodeSize();

	unlock();
}




void* GeneralPool_Impl::alloc(unsigned int size)				
{
	GeneralPool_Page*	page;
	GeneralPool_Node*	node;

	unsigned int realSize = size;

	// 최소 할당 사이즈
	// ( 너무 사이즈가 작은 것들로 Node 가 할당되면 Alloc 시 Search와 Free시 성능에 좋지 않으니깐~ )
	if (size < min_alloc_size)
	{
		size = min_alloc_size;
	}
	
	// 최소 할당 사이즈로 패딩
	size = ((size / min_alloc_size) + (size % min_alloc_size == 0 ? 0 : 1)) * min_alloc_size;

	lock();

	if (size + nodeSize() > default_page_size)
	{
		GeneralPool_Node*	node;

		//===============================================================================================
		// 페이지 크기보다 크므로 할당이 불가능하다 상위( 기본 ) 얼로케이터에서 직접 할당받는다
		// 추후 개선 작업
		//===============================================================================================
		node = (GeneralPool_Node*) _default_allocator.alloc(size + nodeSize());

		node->clear(null);

		node->_size			= size;
		node->_realSize		= realSize;
		node->_allocated	= true;

		unlock();

		return node->getMemory();
	}

	page = _availablePage;

	while (page != null)
	{
		if (page->_availableSize >= size + nodeSize())
		{
			// 연속된 공간으로 존재하는지 확인한다
			node = page->_available;

			while (node != null)
			{
				if (node->_size >= size)
				{
					// 페이지에 남은 공간을 갱신한다
					page->_availableSize -= size + nodeSize();

					// 충분히 큰 노드다, 이 노드를 할당한다
					if (node->_size >= size + nodeSize() + min_alloc_size)
					{
						// 이 노드를 잘라서 새 노드를 만든다
						splitAllocNode(page, node, size);

						unlock();

						// 얼로케이션 크기를 기록한다
						node->_realSize = realSize;

						// 페이지 유효성을 검사;
						validate_page( node->_owner );

						return node->getMemory();
					}
					else
					{
						// 이 노드를 통째로 할당한다

						// 빈 노드 리스트에서 제거한다
						page->removeAvailableList(node);

						unlock();

						// 얼로케이션 크기를 기록한다
						node->_realSize = realSize;
						
						validate_page( node->_owner );

						return node->getMemory();
					}
				}

				node = node->_availableNext;
			}
		}
		else
		{

			page->_miss++;

			
			// 캐쉬 미스율이 너무 높으면 리스트에서 빼고 Missing List에 대기 시킴
			if( page->_miss > max_miss_count )
			{
				GeneralPool_Page* missingPage = page;
				// 현재 리스트에서 때어냄
				if( page->_prev )
					page->_prev->_next = page->_next;
				if( page->_next )
					page->_next->_prev = page->_prev;

				if( page == _availablePage )
					_availablePage = page->_next;

				page = page->_next;

				if( page ) {
					if( page->_next == page ) {
						assert( page->_next == page);
					}
				}

				// missing page 에 붙임
				missingPage->_prev				= null;
				missingPage->_next				= _missingPage;
				if( _missingPage )
					_missingPage->_prev = missingPage;

				_missingPage = missingPage;
				continue;
			}
		}

		// 다음 페이지를 검색한다
		page = page->_next;
	}

	// 할당할 적당한 페이지가 없으므로 새 페이지를 만듬	
	allocPage();

	void* pReturn = null;
	pReturn = alloc(size);
	assert_msg( "새 페이지에 할당할 수 없음", pReturn != null );

	unlock();

	return pReturn;
	
}

void GeneralPool_Impl::splitAllocNode( GeneralPool_Page* page, GeneralPool_Node* node, unsigned int size)
{
	GeneralPool_Node*	pNewBlankNode;
	BYTE*				pReturnAddress;

	// 노드에 사용 가능한 메모리 블럭을 얻어온다
	pReturnAddress = node->getMemory();

	// 새 노드를 할당한다
	pNewBlankNode = (GeneralPool_Node*) (pReturnAddress + size);
	pNewBlankNode->clear(node->_owner);

	assert_msg("노드 할당을 위한 충분하지 않은 공간", node->_size >= size + nodeSize() + min_alloc_size);

	// 새 노드에 사용 가능한 공간을 계산한다
	pNewBlankNode->_size = node->_size - size - nodeSize();

	// 원래 노드에서 새 노드에 할당된 공간을 제거한다
	node->_size = size;

	// 원래 노드를 할당된 상태로 변경한다
	node->_allocated = true;

	// 1. 사용 가능 노드 리스트의 링크를 변경한다
	if (node->_availablePrev == null)
	{
		assert_msg("유효하지 않은 헤드", page->_available == node);

		// 제거된 노드가 사용 가능 노드의 헤더였으므로 업데이트
		page->_available = pNewBlankNode;
		pNewBlankNode->_availablePrev = null;
	}
	else
	{
		node->_availablePrev->_availableNext = pNewBlankNode;
		pNewBlankNode->_availablePrev = node->_availablePrev;
	}

	// 제거되는 노드의 다음 사용 가능 노드를 새 빈 노드의 다음 사용 가능 노드로 연결한다
	pNewBlankNode->_availableNext = node->_availableNext;

	if (node->_availableNext != null)
	{
		node->_availableNext->_availablePrev = pNewBlankNode;
	}

	// 제거된 노드의 링크를 끊는다
	node->_availablePrev = null;
	node->_availableNext = null;

	// 2. 노드 리스트의 링크를 수정한다
	pNewBlankNode->_prev = node;
	pNewBlankNode->_next = node->_next;

	if (node->_next != null)
	{
		node->_next->_prev = pNewBlankNode;
	}

	node->_next = pNewBlankNode;
}

void GeneralPool_Impl::free(void* p)
{
	GeneralPool_Node*	node;

	lock();

	node = Node(p);
	assert_msg("잘못된 노드", node->_allocated);
	assert_msg("잘못된 노드", node->_availablePrev == null);
	assert_msg("잘못된 노드", node->_availableNext == null);

	if (node->_size + nodeSize() > default_page_size)
	{
		// 상위 얼로케이터에서 직접 할당받은 것이다
		_default_allocator.free( (void*&)node);

		unlock();

		return;
	}
	// 페이지 유효성 체크
	validate_page( node->_owner );

	// 할당되었음 플래그를 리셋한다
	node->_allocated = false;

	// 페이지에 남은 공간을 갱신한다
	node->_owner->_availableSize += node->_size + nodeSize();

	// 앞 뒤의 노드와 합칠 필요가 있는지 살펴본다
	if (node->_prev != null)
	{
		if (node->_next != null)
		{
			// 앞 뒤 노드 모두 존재한다
			if (!node->_prev->_allocated)
			{
				// 앞의 노드와 합쳐야 한다
				if (!node->_next->_allocated)
				{
					freeMergePrevAndNext(node);
				}
				else
				{
					freeMergePrev(node);
				}
			}
			else if (!node->_next->_allocated)
			{
				freeMergeNext(node);
			}
			else
			{
				freeMergeNone(node);
			}
		}
		else
		{
			//  앞 노드만 존재한다
			if (!node->_prev->_allocated)
			{
				freeMergePrev(node);
			}
			else
			{
				freeMergeNone(node);
			}
		}
	}
	else
	{
		if (node->_next != null)
		{
			// 뒤 노드만 존재한다
			if (!node->_next->_allocated)
			{
				freeMergeNext(node);
			}
			else
			{
				freeMergeNone(node);
			}
		}
		else
		{
			// 앞 뒤 노드 모두 없다
			freeMergeNone(node);
		}
	}

	assert_msg("availabe가 Null이면 안됨", node->_owner->_available != null);
	validate_page( node->_owner );

	// 캐쉬 Missing Rate이 높아서 missingList에 존재하고 있을 경우 빼서 다시 _availablePrev의 제일 마지막에
	if( node->_owner->_miss > max_miss_count )
	{
		GeneralPool_Page* page = node->_owner;
		assert_msg( "AvaialbePage는 존재 해야 함", _availablePage != null );
		assert_msg( "Missing Count 는 64임", page->_miss == max_miss_count + 1 );
		page->_miss = 0;

		if( page->_prev )
			page->_prev->_next = page->_next;
		if( page->_next )
			page->_next->_prev = page->_prev;
        
		if( _missingPage == page )
		{
			_missingPage = page->_next;
		}

		page->_prev				= null;
		page->_next				= _availablePage;
		if( _availablePage )
			_availablePage->_prev = page;

		_availablePage = page;
	}

	unlock();
}

void GeneralPool_Impl::freeMergePrevAndNext(GeneralPool_Node* node)
{
	GeneralPool_Node*	prevNode;
	GeneralPool_Node*	nextNode;

	prevNode	= node->_prev;
	nextNode	= node->_next;
#ifdef _DEBUG
	assert_msg("잘못된 리스트", prevNode->_next == node);
	assert_msg("잘못된 리스트", nextNode->_prev == node);

	if (prevNode->_availablePrev == null)
	{
		assert_msg("잘못된 리스트", prevNode->_owner->_available == prevNode);
	}

	// 사용 가능 노드 리스트에서 뒤쪽 노드를 제거한다
	assert_msg("잘못된 리스트", prevNode->_availableNext == nextNode);
	assert_msg("잘못된 리스트", nextNode->_availablePrev == prevNode);
#endif
	prevNode->_availableNext = nextNode->_availableNext;
	if (nextNode->_availableNext != null)
	{
		nextNode->_availableNext->_availablePrev = prevNode;
	}

	// 노드 리스트에서 프리된 노드와 뒤쪽 노드를 없앤다
	prevNode->_next = nextNode->_next;
	if (nextNode->_next != null)
	{
		nextNode->_next->_prev = prevNode;
	}

	// 노드 크기를 조정한다
	prevNode->_size += node->_size + nodeSize() +	nextNode->_size + nodeSize();

}

void GeneralPool_Impl::freeMergePrev( GeneralPool_Node* node)

{
	GeneralPool_Node*	prevNode;

	prevNode	= node->_prev;

	assert_msg("잘못된 리스트", prevNode->_next == node);

#ifdef _DEBUG
	if (prevNode->_availablePrev == null)
	{
		assert_msg("잘못된 리스트", prevNode->_owner->_available == prevNode);
	}
#endif

	// 노드 리스트에서 프리된 노드를 없앤다
	prevNode->_next = node->_next;

	if (node->_next != null)
	{
		node->_next->_prev = prevNode;
	}

	// 노드 크기를 조정한다
	prevNode->_size += node->_size + nodeSize();

}

void GeneralPool_Impl::freeMergeNext(GeneralPool_Node* node)
{
	GeneralPool_Node*	nextNode;

	nextNode	= node->_next;

	assert_msg("잘못된 리스트", nextNode->_prev == node);

#ifdef _DEBUG
	if (nextNode->_availablePrev == null)
	{
		assert_msg("잘못된 리스트", nextNode->_owner->_available == nextNode);
	}
#endif

	// 사용 가능 노드 리스트에서 뒤쪽 노드를 새로 프리된 노드로 대체한다
	if (nextNode->_availableNext != null)
	{
		nextNode->_availableNext->_availablePrev = node;
		node->_availableNext = nextNode->_availableNext;
	}

	if (nextNode->_availablePrev != null)
	{
		nextNode->_availablePrev->_availableNext = node;
		node->_availablePrev = nextNode->_availablePrev;
	}
	else
	{
		GeneralPool_Page*	page;
		page = node->_owner;

		// 뒤쪽 노드가 가장 앞쪽 사용 가능 노드이므로 리스트 헤드를 교체
		assert_msg("잘못된 리스트 헤드", page->_available == nextNode);
		page->_available = node;
	}

	// 노드 리스트에서 뒤쪽 노드를 없앤다
	node->_next = nextNode->_next;
	if (nextNode->_next != null)
	{
		nextNode->_next->_prev = node;
	}

	// 노드 크기를 조정한다
	node->_size += nextNode->_size + nodeSize();

}

void GeneralPool_Impl::freeMergeNone(GeneralPool_Node* node)
{
	GeneralPool_Page*	page;
	GeneralPool_Node*	prevNode;
	bool					bLinked = false;

	page = node->_owner;

	// 단지 사용 가능 노드 리스트에만 업데이트한다
	if (page->_available != null)
	{
		// 앞쪽에서 가장 먼저 사용 가능한 노드를 찾는다
		prevNode = node->_prev;

		while (prevNode != null)
		{
			if (!prevNode->_allocated)
			{
				// 사용 가능한 노드를 찾았다, 리스트에 끼워넣는다
				if (prevNode->_availableNext != null)
				{
					prevNode->_availableNext->_availablePrev = node;
					node->_availableNext = prevNode->_availableNext;
				}

				prevNode->_availableNext = node;
				node->_availablePrev = prevNode;

				bLinked = true;
				break;
			}

			prevNode = prevNode->_prev;
		}

		if (!bLinked)
		{
			// 앞쪽으로 빈 노드가 없다, 즉 이 노드가 가장 앞쪽 빈 노드가 된다
			page->_available->_availablePrev = node;
			node->_availableNext = page->_available;

			page->_available = node;
		}
	}
	else
	{
		// 이 노드가 사용 가능한 유일한 노드가 된다
		page->_available = node;
		
	}
}


void GeneralPool_Impl::valid(GeneralPool_Page* page)
{
	GeneralPool_Node*	node;
	GeneralPool_Node*	pLastAvailableNode = null;

	// 첫 노드부터 탐색한다
	node = page->_firstNode;

	while (node != null)
	{
		if (!node->_allocated)
		{
			assert_msg("잘못된 할당 리스트", node->_availablePrev == pLastAvailableNode);
			assert_msg("잘못된 할당 리스트", node->_availablePrev != node);
			assert_msg("잘못된 할당 리스트", node->_availableNext != node);

			if (node->_availablePrev == null)
			{
				assert_msg("잘못된 Root", node->_owner->_available == node);
			}

			pLastAvailableNode = node;
		}
		node = node->_next;
	}
}