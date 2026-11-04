/*
	IEnumerator.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	01/04/2005
*/

#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace System
{
namespace Collections
{
	
template<typename T>
class IEnumerator
{
public:
	
	// Retrieves the current element in the collection
	virtual T& GetCurrent() const = 0;

	// Advances the enumerator to the next element of the collection
	virtual bool MoveNext() = 0;

	// Sets the enumerator to its initial position, which is before the first element in the collection
	virtual void Reset() = 0;
};

template<typename T, typename BackingStorage>
class CStlEnumerator
	: public CObject
	, public IEnumerator<T>
{
public:

	StlEnumerator& operator= (const StlEnumerator& rhs)
	{
		if (rhs != *this)
		{
			*this = rhs;
		}

		return *this;
	}
	
	// Retrieves the current element in the collection
	virtual T& GetCurrent() const
	{
		return (*m_Current);
	}

	// Advances the enumerator to the next element of the collection
	virtual bool MoveNext()
	{
		if( m_Reset )
		{
			m_Reset = false;
			m_Current = m_refCollection.begin( );
			return ( m_Current != m_refCollection.end( ) );
		}

		if( m_Current == m_refCollection.end( ) )
		{
			return false;
		}
		else
		{
			++m_Current;
			if( m_Current == m_refCollection.end( ) )
			{
				return false;
			}			
		}
		return true;
	}

	// Sets the enumerator to its initial position, which is before the first element in the collection
	virtual void Reset()
	{
		m_Reset = true;
	}


	CStlEnumerator( typename BackingStorage& Collection )
		: m_refCollection( Collection )
		, m_Reset( true )
	{
	}
	virtual ~CStlEnumerator()
	{
	}

protected:

	typename BackingStorage&			m_refCollection;	// reference to parent list
	typename BackingStorage::iterator   m_Current;			// the current element
	bool								m_Reset;			// flags that we are one element before the beginning of the set
};


}	// Collections
}	// System
}	// WT