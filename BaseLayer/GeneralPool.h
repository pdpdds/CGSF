#pragma once
#include <windows.h>


	class iMemPool
	{
	public:
		virtual				~iMemPool() {}
		virtual void*		alloc(unsigned int n) = 0;
		virtual void		free(void*& p) = 0;
	};

	/**
		@class Runtime Memory Pool v0.1
		@brief 메모리 Page를 유저레벨에서 관리하여 동적인 사이즈의 메모리를 할당함
	 */

	class RtMemPool : public iMemPool
	{
	public:
		virtual void*		alloc(unsigned int n);
		virtual void		free(void*& p);
	};

	enum
	{
		// 기본 페이지 사이즈
		default_page_size	=	1024 * 16,
		// 실패수
		max_miss_count		=	8,
		// 최소 할당 사이즈
		min_alloc_size		=	32
	};

	class GeneralPool_Impl;

	/**
		@class 범용 할당자 v0.1
		@brief 메모리 Page를 유저레벨에서 관리하여 동적인 사이즈의 메모리를 할당함
		
		@remark 
			메모리가 할당될때 32byte 이상의 메모리 공간이 할당된다. 그러므로 너무 작은 메모리를 할당할때 사용하면 좋지 않음
			메모리 할당시 page size는 최소 4k로 설정되며 Page size(32k) 를 넘어가는 메모리 공간에 대해서는 기본 할당자가 사용되어 진다. 
			페이지와 메모리 노드는 선형 관리되어 지며 할당을 위한 적절한 페이지를 찾기 위해 선형 알고리즘이 사용되어진다.

		@date 2008/01
		@author kkkkkkman
		@code
		GeneralPool m;

		std::vector< void* > v;

		for( int i=0;i<1000;++i)
			v.push_back( m.alloc( 20 ) );

		for( int i=0;i<1000;++i)
			m.free( v[i]) ;
		@endcode

	 */
	class GeneralPool : public iMemPool
	{
		GeneralPool_Impl*	m_impl;
	public:
		
	public:
							GeneralPool();
							~GeneralPool();

		// 메모리를 할당받는다
		virtual void*		alloc(unsigned int n);
		// 메모리를 해제한다
		virtual void		free(void*& p);

		// 
		static unsigned int	size(const void* p);
	};

