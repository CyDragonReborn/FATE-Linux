/*
	SSE.h

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	3/9/2006

*/

#pragma once

#include <malloc.h>

namespace WT
{
namespace System
{

int32 const KSSEAlignmentSize = 16;

//Are SSE instructions supported?
//TODO we should have a static object that initializes a global variable.
inline bool __cdecl IsSSESupported();

inline bool WTCall IsSSEAligned( void const* pT )
{
	return !(reinterpret_cast<ptrdiff_t>( pT ) & (KSSEAlignmentSize - 1));
}

//SSE-align the pointer by advancing it zero or more bytes
inline void* WTCall SSEAlign( void* Ptr )
{
	void* Result = reinterpret_cast<void*>(
		(reinterpret_cast<ptrdiff_t>( Ptr ) + (KSSEAlignmentSize - 1)) & (KSSEAlignmentSize - 1) );
	assert( IsSSEAligned( Result) );
	return Result;
}

//An allocator that allocates on N-byte boundaries.
//Due to the implementation, N must be a power of two. Also, the allocated
//objects must be a multiple of size N in order for all of the objects to be 
//aligned.
template <int N, typename T> struct AlignedAllocator : std::allocator<T>
{
	pointer allocate(size_type n, const void *hint = NULL )
	{
		hint;
		assert( Math_IsPow2( N ) );
		assert( !( sizeof( T ) % N ) );
		return static_cast<pointer>( _aligned_malloc( n * sizeof( value_type ), N ) );
	}

    void deallocate(pointer p, size_type n)
	{
		n;
		assert( Math_IsPow2( N ) );
		assert( !( sizeof( T ) % N ) );
		_aligned_free( p );
	}

	template <class _Other> struct rebind
	{
		typedef AlignedAllocator<N, _Other> other;
	};
};

//An allocator that aligns for SSE aligned vector access. The allocated objects 
//must be a multiple of size N in order for all of the objects to be aligned.
template <typename T> struct SSEAllocator : AlignedAllocator<16, T>
{
	template <class _Other> struct rebind
	{
		typedef SSEAllocator<_Other> other;
	};
};

inline bool __cdecl IsSSESupported()
{
	static bool bResult = false;
	static bool bInit   = false;

	if (!bInit)
	{
		//Check for SSE support
		int32 Result;
		__asm
		{
			sub eax,eax
			inc eax
			cpuid
			mov Result, edx
		}

		bResult = Result & (1 << 25) ? true : false;
		bInit = true;
	}

	return bResult;
}

}
}
