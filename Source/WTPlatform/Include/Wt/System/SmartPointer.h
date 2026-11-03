/***************************************************************************************
 *
 * SmartPointer.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          8/10/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include <malloc.h>
#include <functional>
#include "WT/System/Platform.h"
#include "WT/System/ReferenceCounted.h"

namespace WT
{
namespace System
{

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

//CHasReferenceCount<T>::Result is WT::TrueType if type T is CReferenceCounted-derived and 
//thus supplies the needed AddRef() and Release() methods with reference-counting 
//semantics.
template <typename T> struct CHasReferenceCount : CCanCastPointer<T, CReferenceCounted const>
{
};

template <typename T> class CIntrusiveOwner
{
public:
	void Release( T const* pObj )
	{
		if (pObj != NULL)
		{
			pObj->Release();
		}
	}

	void AddRef( T const* pObj )
	{
		if (pObj != NULL)
		{
#ifndef NDEBUG
			//If an assertion fails, then the object wasn't constructed with operator new,
			//but is instead a stack object or a member variable of another object. This
			//object isn't compatible with the reference counting mechanism.
			assert( pObj->IsNewedObject() );
#endif

			pObj->AddRef();
		}
	}

	void SetOwnership( CIntrusiveOwner const& )
	{
	}

	bool IsCompatible( CIntrusiveOwner const& ) const
	{
		return true;
	}
};

template <typename T> class CExternalOwner
{
public:
	CExternalOwner()
		: m_pRefCount( NULL )
	{
	}

	~CExternalOwner()
	{
	}

	void Release( T const* pObj )
	{
		if (m_pRefCount != NULL)
		{
			LONG Result = ::InterlockedDecrement( reinterpret_cast<LONG*>( m_pRefCount ) );
			assert( Result >= 0 );
			if (!Result)
			{
				delete pObj;
				_aligned_free( m_pRefCount );
				m_pRefCount = NULL;
			}
		}
	}

	void AddRef( T const* pObj )
	{
		if (m_pRefCount == NULL)
		{
			if (pObj != NULL)
			{
				m_pRefCount = static_cast<int32*>( _aligned_malloc( sizeof int32, 4 ) );
				*m_pRefCount = 1;
			}
		} else {
			::InterlockedIncrement( reinterpret_cast<LONG*>( m_pRefCount ) );
		}
	}

	void SetOwnership( CExternalOwner const& Owner )
	{
		m_pRefCount = Owner.m_pRefCount;
	}

	bool IsCompatible( CExternalOwner const& Owner ) const
	{
		return m_pRefCount == Owner.m_pRefCount;
	}

private:
	int32* m_pRefCount;
};

//SmartPointerConversion provides smart pointer functionality dependent on the template
//argument, which tells whether implicit conversions to the raw pointer are allowed. 
template <typename T, typename AllowConversions> class SmartPointerConversion;

template<typename T> class SmartPointerConversion<T, TrueType>
{
public:
	operator T*() const;
	operator void*() const; //This second conversion prevents using delete directly on the SmartPointer.
protected:
	T* m_pObj;
};

template<typename T> class SmartPointerConversion<T, FalseType>
{
	class CTester
	{
		void operator delete( void* );
	};

public:
	operator CTester*() const; //This conversion supports "if (sp)"

protected:
	T* m_pObj;
};

template <typename T, typename HasReferenceCount> class COwner;
template <typename T> class COwner<T, FalseType> : public CExternalOwner<T> {};
template <typename T> class COwner<T, TrueType > : public CIntrusiveOwner<T> {};

//A smart pointer that has ownership over the referrent and is suitable
//for use in STL containers.
//
//It is recommend when you create an object, that you store it in a
//SmartPointer to manage ownership over the object.
//
//Ownership is currently implemented via the intrusive reference count in
//CObject-derived types, and via an external reference count for other types.
template <typename T> class SmartPointer : public SmartPointerConversion< T, typename CHasReferenceCount<T>::Result >
{
public:
	//
	// Constructors
	//
	SmartPointer();
	SmartPointer( T* pObj );
	template <typename U> SmartPointer( U* pObj );
	SmartPointer( SmartPointer<T> const& Ptr );
	template <typename U> SmartPointer( SmartPointer<U> const& Ptr );

	//
	// Destructor
	//
	~SmartPointer();

	//
	// Assignment operators
	//
	SmartPointer<T>& operator=( T* pObj );
	template <typename U> SmartPointer<T>& operator=( U* pObj );
	SmartPointer<T>& operator=( SmartPointer<T> const& Ptr );
	template <typename U> SmartPointer<T>& operator=( SmartPointer<U> const& Ptr );
	
	//
	// Accessors
	//
	T& operator*() const;
	T* operator->() const;
	T* get() const; //Get the pointer value. Equivalent to &*.

	//
	// Comparison operators
	//
	bool operator!() const;
	bool operator==( T* pObj ) const;
	bool operator==( SmartPointer<T> const& Ptr ) const;
	template<typename U> bool operator==( U* pObj ) const;
	template<typename U> bool operator==( SmartPointer<U> const& Ptr ) const;
	bool operator!=( T* pObj ) const;
	bool operator!=( SmartPointer<T> const& Ptr ) const;
	template<typename U> bool operator!=( U* pObj ) const;
	template<typename U> bool operator!=( SmartPointer<U> const& Ptr ) const;

private:
	COwner<T, typename CHasReferenceCount<T>::Result> m_Owner;

	friend class SmartPointer;
};

}
}

template<typename T> struct std::less< WT::System::SmartPointer<T> >
	: binary_function< WT::System::SmartPointer<T>, WT::System::SmartPointer<T>, bool>
{
	result_type operator()( first_argument_type const& Left, second_argument_type const& Right ) const
	{
		return std::less<T*>()( Left.get(), Right.get() );
	}
};

#include "WT/System/SmartPointer.inl"
