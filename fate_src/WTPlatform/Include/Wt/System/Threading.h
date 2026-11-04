//
//	Threading.h
//
//	Copyright (C) 2005-6 WildTangent, Inc. 
//	All Rights Reserved
//

#pragma once

#include "windows.h"

namespace WT
{
namespace System
{

// Provides a wrapper mechanism for Win32 CRITICAL_SECTION objects
class CCriticalSection
{
public:

	inline CCriticalSection()
	{
		::InitializeCriticalSection( &m_CriticalSection );
	}

	inline ~CCriticalSection()	
	{
		::DeleteCriticalSection ( &m_CriticalSection );		
	}

	inline void Lock()
	{
		::EnterCriticalSection( &m_CriticalSection );
	}

	inline void Unlock()
	{
		::LeaveCriticalSection( &m_CriticalSection );
	}

	inline LPCRITICAL_SECTION operator()()
	{
		return &m_CriticalSection;
	}

private:

	CRITICAL_SECTION m_CriticalSection;
};

// Provides a scoped auto lock/unlock mechanism for Win32 CRITICAL_SECTION objects
// To maximize speed, validation on the provided CRITICAL_SECTION pointer is not performed
class CAutoCriticalSection
{
public:

	inline CAutoCriticalSection( LPCRITICAL_SECTION lpCriticalSection )
		: m_lpCriticalSection( lpCriticalSection )
	{
		::EnterCriticalSection( m_lpCriticalSection );
	}

	inline ~CAutoCriticalSection()	
	{
		::LeaveCriticalSection( m_lpCriticalSection );
	}

private:

	LPCRITICAL_SECTION m_lpCriticalSection;
};

//Like CAutoCriticalSeciton, call class T's Lock and Unlock automatically.
//It assumes you have a class with Lock and Unlock function.
//  class MyClass {
//  public:
//      void Lock();
//      void Unlock();
//  };
//To use: 
//  MyClass* pobj = new MyClass;
//  {
//      CAutoClassLock<MyClass> autolock(pobj);
//      ...
//  }
//  
//Added by Rui 4/5/2006

template<class T> class CAutoClassLock
{
public:
	CAutoClassLock(T* p)
	{
		m_p = p;
		m_p->Lock();
	}
	~CAutoClassLock()
	{
		m_p->Unlock();
	}
protected:
	T* m_p;
};

} // namespace System
} // namespace WT
