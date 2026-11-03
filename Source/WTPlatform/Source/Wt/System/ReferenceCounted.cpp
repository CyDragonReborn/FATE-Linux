#include "WT/stdafx.h"

#include "windows.h"
#include "ReferenceCounted.h"

namespace WT
{
namespace System
{

CReferenceCounted::CReferenceCounted()
	: m_ReferenceCount( 0 )
{
}

/*virtual*/ CReferenceCounted::~CReferenceCounted()
{
	assert( !m_ReferenceCount );
}

void CReferenceCounted::AddRef() const
{
	::InterlockedIncrement( reinterpret_cast<LONG*>( &const_cast<int32&>( m_ReferenceCount ) ) );
}

void CReferenceCounted::Release() const
{
	LONG Result = ::InterlockedDecrement( reinterpret_cast<LONG*>( &const_cast<int32&>( m_ReferenceCount ) ) );
	assert( Result >= 0 );
	if (!Result)
	{
		delete this;
	}
}

#ifndef NDEBUG

CCriticalSection* CReferenceCounted::gm_pNewedObjectsCS;
std::set<CReferenceCounted const*>* CReferenceCounted::gm_pNewedObjects;

/*static*/ void WTCall CReferenceCounted::RegisterNewedObject( CReferenceCounted const* pObj )
{
	assert( pObj != NULL );

	if (gm_pNewedObjectsCS == NULL)
	{
		gm_pNewedObjectsCS = new CCriticalSection(); //This will leak...
	}

	CAutoClassLock<CCriticalSection> Lock( gm_pNewedObjectsCS );

	if (gm_pNewedObjects == NULL)
	{
		gm_pNewedObjects = new std::set<CReferenceCounted const*>();
	}

	std::pair<std::set<CReferenceCounted const*>::iterator, bool> Result = gm_pNewedObjects->insert( pObj );
	assert( Result.second );
}

/*static*/ void WTCall CReferenceCounted::UnregisterNewedObject( CReferenceCounted const* pObj )
{
	assert( pObj != NULL );

	assert( gm_pNewedObjectsCS != NULL );
	CAutoClassLock<CCriticalSection> Lock( gm_pNewedObjectsCS );

	assert( gm_pNewedObjects != NULL );
	std::set<CReferenceCounted const*>::iterator it = gm_pNewedObjects->find( pObj );
	assert( it != gm_pNewedObjects->end() );
	gm_pNewedObjects->erase( it );

	if (gm_pNewedObjects->empty())
	{
		delete gm_pNewedObjects;
		gm_pNewedObjects = NULL;
	}
}

bool CReferenceCounted::IsNewedObject() const
{
	assert( gm_pNewedObjectsCS != NULL );
	CAutoClassLock<CCriticalSection> Lock( gm_pNewedObjectsCS );

	if (gm_pNewedObjects == NULL)
	{
		return false;
	}

	return gm_pNewedObjects->find( this ) != gm_pNewedObjects->end();
}

/*static*/ void* WTCall CReferenceCounted::operator new( size_t size )
{
	void* pResult = ::new uint8[size];
	RegisterNewedObject( static_cast<CReferenceCounted*>( pResult ) );
	return pResult;
}

/*static*/ void WTCall CReferenceCounted::operator delete( void* pMem )
{
	UnregisterNewedObject( static_cast<CReferenceCounted*>( pMem ) );
	::delete[] pMem;
}

//Placement new/delete operators
/*static*/ void* WTCall CReferenceCounted::operator new( size_t size, void* Ptr )
{
	size;
	return Ptr;
}

/*static*/ void WTCall CReferenceCounted::operator delete( void* pMem, void* Ptr )
{
	pMem;
	Ptr;
}

#endif

}
}
