/*
	List.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	01/04/2005
*/

#pragma once

#include "WT/System/Collections/IEnumerable.h"
#include "WT/System/SystemException.h"
#include <list>

namespace WT
{
namespace System
{
namespace Collections
{

template<typename T>
class CList 
	: public CObject
	, public IEnumerable<T>
{
public:

	typedef StlEnumerator<T, std::list<T> > StlListEnumerator;

	// Returns an enumerator that can iterate through a collection
	virtual IEnumerator<T>* GetEnumerator()
	{
		return static_cast<IEnumerator<T>* >( new StlListEnumerator( m_List ) );
	}
	
	// Retrieves the number of elements contained in the collection
	virtual uint32 GetCount() const
	{
		return static_cast<uint32>(m_List.size());
	}

	// Sets the number of elements contained in the collection
	virtual void SetCount( uint32 Count )
	{
		if (Count >= m_List.size())
		{
			while (Count > m_List.size())
			{
				m_List.push_back( T() );
			}
		} else {
			while (Count < m_List.size())
			{
				m_List.pop_back();
			}
		}
	}

	// Retrieves the element at the specified index
	virtual T GetItem( uint32 Index ) const
	{
		uint32 i = 0;
		std::list<T>::const_iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( i == Index )
			{
				return *it;
			}
			++i;
			++it;
		}
		return T();
	}

	// Sets the element at the specified index
	virtual void SetItem( uint32 Index, T& Item )
	{
		uint32 i = 0;
		std::list<T>::iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( i == Index )
			{
				*it = Item;
				return;
			}
			++i;
			++it;
		}
	}

	// Adds an item to the back (top) of the list 
	// This method returns the 0-based index the element was inserted
	virtual uint32 AddBack( T& Item )
	{
		m_List.push_back( Item );
		return static_cast<uint32>(m_List.size()) - 1;
	}

	// Adds an item to the front (bottom) of the list
	virtual void AddFront( T& Item )
	{
		m_List.push_front( Item );
	}

	// Returns the back (most recent) item
	virtual T GetBack() const
	{
		return m_List.back();
	}

	// Returns the front (least recent) item
	virtual T GetFront() const
	{
		return m_List.front();
	}

	// Removes the back (most recent) item from the list
	virtual void PopBack()
	{
		m_List.pop_back();
	}

	// Removes the front (least recent) item from the list
	virtual void PopFront()
	{
		m_List.pop_front();
	}

	// Removes all items from the list
	virtual void Clear() 
	{
		m_List.clear();
	}

	// Determines whether the list is empty
	virtual bool IsEmpty() const
	{
		return (this->GetCount() == 0);
	}

	// Determines whether the list contains a specific value
	virtual bool Contains( const T& Item ) const
	{
		std::list<T>::const_iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( *it == Item )
			{
				return true;
			}
			++it;
		}
		return false;
	}

	// Determines the index of a specific item in the list
	virtual int32 GetIndexOf( T& Item ) const
	{
		int32 i = 0;
		std::list<T>::const_iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( *it == Item )
			{
				return i;
			}
			++i;
			++it;
		}
		return -1;
	}

	// Inserts an item to the list at the specified position
	// If Index equals the number of items in the list, then the value is appended to the end
	virtual bool Insert( T& Item, uint32 Index )
	{
		if( Index == GetCount() )
		{
			AddBack( Item );
			return true;
		}

		uint32 i = 0;
		std::list<T>::iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( i == Index )
			{
				m_List.insert( it, Item );
				return true;
			}
			++i;
			++it;
		}
		return false;	
	}

	// Removes the first occurrence of a specific object from the list 
	virtual bool Remove( T& Item )
	{
		std::list<T>::iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( *it == Item )
			{
				m_List.erase( it );				
				return true;
			}
			++it;
		}
		return false;
	}

	// Removes the list item at the specified index
	virtual bool RemoveAt( uint32 Index )
	{
		uint32 i = 0;
		std::list<T>::iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( i == Index )
			{
				m_List.erase( it );				
				return true;
			}
			++i;
			++it;
		}
		return false;
	}

	// Copies the elements of the list to the specified STL list, starting at the given index
	virtual void CopyTo( std::list<T>& Array, int32 StartIndex ) const
	{
		if( StartIndex < 0 )
		{
			throw WT::System::ArgumentOutOfRangeException( "Index was less than zero" );
		}

		if( m_List.size() >= static_cast<uint32>(StartIndex) )
		{
			throw WT::System::ArgumentException( "The number of elements in the source collection is greater than the available space from index to the end of the destination array" );
		}

		int32 i = 0;
		std::list<T>::const_iterator it = m_List.begin();
		while( it != m_List.end() )
		{
			if( i >= StartIndex )
			{
				Array.push_back( *it );
			}
			++i;
			++it;
		}		
	}

	virtual void CopyFrom( const std::list<T>& STLList, int32 StartIndex )
	{
		if( StartIndex < 0 )
		{
			throw WT::System::ArgumentOutOfRangeException( "Index was less than zero" );
		}

		if( STLList.size() >= static_cast<uint32>(StartIndex) )
		{
			throw WT::System::ArgumentException( "The number of elements in the source collection is greater than the available space from index to the end of the destination array" );
		}

		int32 i = 0;
		std::list<T>::const_iterator it = STLList.begin();
		while( it != STLList.end() )
		{
			if( i >= StartIndex )
			{
				m_List.push_back( *it );
			}
			++i;
			++it;
		}
	}

	CList<T>( )
	{
	}
	CList<T>( const List<T>& Other )
	{
		CopyFrom(Other.m_List, 0);
	}
	virtual ~CList<T>( )
	{
	}

protected:

	std::list<T> m_List;
};

}	// Collections
}	// System
}	// WT
