/***************************************************************************************
 *
 * ReferenceCounted.h
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

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Platform.h"
#include "WT/System/Threading.h"
#include <set> //Needed ifndef NDEBUG only

namespace WT
{
namespace System
{

class CReferenceCounted
{
public:
	virtual ~CReferenceCounted();

	//Increase the reference count by 1. Must be followed by a corresponding call to Release().
	void AddRef() const;

	//Decrease the reference count by 1. Must correspond to an earlier call to AddRef(). If
	//the reference count falls to zero, the object will be deleted.
	void Release() const;

protected:
	CReferenceCounted();

private:
	__declspec( align( 4 ) ) int32 m_ReferenceCount;

#ifndef NDEBUG
public:
	//Report true if the object was new'd using operator new. Report false otherwise
	//(e.g. if the object was created with operator new[], is on the stack, or is a member
	//variable).
	bool IsNewedObject() const;

	//Override operators new and delete for this class. The purpose is to track which objects
	//are newed. This allows SmartPointer.h to quickly detect dangerous attempts to create a 
	//smart pointer to an object that resides on the stack or is a member variable.
	static void* WTCall operator new( size_t size );
	static void WTCall operator delete( void* pMem );

	//Placement new/delete operators
	static void* WTCall operator new( size_t size, void* Ptr );
	static void WTCall operator delete( void* pMem, void* Ptr );

	//Define MFC-related operators that work with DEBUG_NEW. These are defined as templates to defer
	//compilation - these depend on MFC global operators that might not be available.
	template<typename T> static void* WTCall operator new( size_t size, T const lpszFileName[], int nLine )
	{
		void* pResult = ::new(lpszFileName, nLine) uint8[size];
		RegisterNewedObject( static_cast<CReferenceCounted*>( pResult ) );
		return pResult;
	}
	
	template<typename T> static void WTCall operator delete( void* pMem, T const lpszFileName[], int nLine )
	{
		UnregisterNewedObject( static_cast<CReferenceCounted*>( pMem ) );
		::operator delete[]( pMem, lpszFileName, nLine );
	}

private:
	//Keep a list of new'd CReferenceCounted-derived objects. Only elements found in this list 
	//are acceptable for storing in a reference-counting SmartPointer - objects not found in 
	//this list were constructed in another way (on the stack or as member variables of 
	//other objects) and can't be deleted properly with the reference-counting mechanism.
	static std::set<CReferenceCounted const*>* gm_pNewedObjects;
	static CCriticalSection* gm_pNewedObjectsCS;

	static void WTCall RegisterNewedObject( CReferenceCounted const* pObj );
	static void WTCall UnregisterNewedObject( CReferenceCounted const* pObj );
#endif
};

}
}
