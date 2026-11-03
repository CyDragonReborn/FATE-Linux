/***************************************************************************************
 *
 * SmartPointer.inl
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          4/24/2006
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

#include "WT/System/SmartPointer.h"

namespace WT
{
namespace System
{

template <typename T> SmartPointerConversion<T, TrueType>::operator T*() const
{
	return m_pObj;
}

template <typename T> SmartPointerConversion<T, TrueType>::operator void*() const
{
	return m_pObj;
}

template<typename T> SmartPointerConversion<T, FalseType>::operator CTester*() const
{
	static CTester g_Tester;
	return m_pObj ? &g_Tester : NULL;
}

template <typename T> SmartPointer<T>::SmartPointer() 
{
	m_pObj = NULL;
}

template <typename T> SmartPointer<T>::SmartPointer( T* pObj )
{
	m_pObj = pObj;
	m_Owner.AddRef( m_pObj );
}

template <typename T> template <typename U> SmartPointer<T>::SmartPointer( U* pObj )
{
	m_pObj = pObj;
	m_Owner.AddRef( m_pObj );
}

template <typename T> SmartPointer<T>::SmartPointer( SmartPointer<T> const& Ptr ) 
{
	m_pObj = Ptr.get();
	m_Owner.SetOwnership( Ptr.m_Owner );
	m_Owner.AddRef( m_pObj );
}

template <typename T> template <typename U> 
	SmartPointer<T>::SmartPointer( SmartPointer<U> const& Ptr ) 
{
	m_pObj = Ptr.get();
	m_Owner.SetOwnership( Ptr.m_Owner );
	m_Owner.AddRef( m_pObj );
}

template <typename T> SmartPointer<T>& SmartPointer<T>::operator=( T* pObj )
{
	if (m_pObj != pObj)
	{
		m_Owner.Release( m_pObj );
		m_pObj = pObj;
		m_Owner.AddRef( m_pObj );
	}
	return *this;
}

template <typename T> template <typename U>
	SmartPointer<T>& SmartPointer<T>::operator=( U* pObj )
{
	if (m_pObj != pObj)
	{
		m_Owner.Release( m_pObj );
		m_pObj = pObj;
		m_Owner.AddRef( m_pObj );
	}
	return *this;
}

template <typename T> SmartPointer<T>& SmartPointer<T>::operator=( SmartPointer<T> const& Ptr )
{
	T* const pObj = Ptr.get();
	if (m_pObj != pObj)
	{
		m_Owner.Release( m_pObj );
		m_pObj = pObj;
		m_Owner.SetOwnership( Ptr.m_Owner );
		m_Owner.AddRef( m_pObj );
	} else {
		/*
		Source and destination both have ownership over same pointer.
		If this assertion fails, then two SmartPointers were created
		independently to have incompatible ownership mechanisms for a 
		single object. This will most likely result in premature and/or
		multiple destruction crashes later. It's better to catch this
		early.
		*/
		assert( m_Owner.IsCompatible( Ptr.m_Owner ) );
	}

	return *this;
}

template <typename T> template <typename U>
	SmartPointer<T>& SmartPointer<T>::operator=( SmartPointer<U> const& Ptr )
{
	T* const pObj = Ptr.get();
	if (m_pObj != pObj)
	{
		m_Owner.Release( m_pObj );
		m_pObj = pObj;
		m_Owner.SetOwnership( Ptr.m_Owner );
		m_Owner.AddRef( m_pObj );
	} else {
		/* 
		Source and destination both have ownership over same pointer.
		If this assertion fails, then two SmartPointers were created
		independently to have incompatible ownership mechanisms for a 
		single object. This will most likely result in premature and/or
		multiple destruction crashes later. It's better to catch this
		early.
		*/
		assert( m_Owner.IsCompatible( Ptr.m_Owner ) );
	}

	return *this;
}

template <typename T> SmartPointer<T>::~SmartPointer()
{
	m_Owner.Release( m_pObj );
}

template <typename T> T& SmartPointer<T>::operator*() const
{
	return *m_pObj;
}

template <typename T> T* SmartPointer<T>::operator->() const
{
	return m_pObj;
}

template <typename T> T* SmartPointer<T>::get() const
{
	return m_pObj;
}

template<typename T> bool SmartPointer<T>::operator!() const
{
	return m_pObj == NULL;
}

template<typename T> bool SmartPointer<T>::operator==( T* pObj ) const
{
	return m_pObj == pObj;
}

template<typename T> bool SmartPointer<T>::operator==( SmartPointer<T> const& Ptr ) const
{
	return m_pObj == Ptr.m_pObj;
}

template<typename T> template<typename U> bool SmartPointer<T>::operator==( U* pObj ) const
{
	return m_pObj == pObj;
}

template<typename T> template<typename U> bool SmartPointer<T>::operator==( SmartPointer<U> const& Ptr ) const
{
	return m_pObj == Ptr.m_pObj;
}

template<typename T> bool SmartPointer<T>::operator!=( T* pObj ) const
{
	return m_pObj != pObj;
}

template<typename T> bool SmartPointer<T>::operator!=( SmartPointer<T> const& Ptr ) const
{
	return m_pObj != Ptr.m_pObj;
}

template<typename T> template<typename U> bool SmartPointer<T>::operator!=( U* pObj ) const
{
	return m_pObj != pObj;
}

template<typename T> template<typename U> bool SmartPointer<T>::operator!=( SmartPointer<U> const& Ptr ) const
{
	return m_pObj != Ptr.m_pObj;
}

}
}
