/*
	llist.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

	CHANGE HISTORY

	Travis Baldree
	7/14/2004		-		added a method to count the nodes in the list

*/


#ifndef _LLIST_H
#define _LLIST_H

#include <stdio.h>
#include "utilities.h"
#include "macros.h"

// this is a templatized linked list

// CLLNode - container where objects in the linked list are stored
template <class T>
class CLLNode
{
public:
	CLLNode( T* pData ) :
			 m_pData( pData ),
			 m_pNext( NULL ),
			 m_pLast( NULL )
	{
	} 

	CLLNode* Next()
	{
		return( m_pNext );
	}
	CLLNode* Last()
	{
		return( m_pLast );
	}
	void SetNext( CLLNode* pNode )
	{
		m_pNext = pNode;
	}
	void SetLast( CLLNode* pNode )
	{
		m_pLast = pNode;
	}
	T* Data()
	{
		return( m_pData );
	}
	void SetData( T* pData )
	{
		m_pData = pData;
	};


private:
	T*		m_pData;
	CLLNode<T>*m_pNext;
	CLLNode<T>*m_pLast;
}; // end CLLNode


// the linked list class
template <class T>
class CLList
{
public:
	CLList( void ) :
			m_pHead( NULL )
	{
	}

	~CLList( void )
	{
		Destroy();
	}

	// count the number of nodes in the list
	uint32 Nodes( void )
	{

		CLLNode<T> *pNode;
		uint32 NodeCount( 0 );

		pNode = m_pHead;
		while ( pNode != NULL )
		{
			NodeCount++;
			pNode = pNode->Next();
		} // end while

		return NodeCount;
	}

	// member functions for manipulating the list
	T* Queue( T* pData )
	{
		CLLNode<T>* pNewNode;

		pNewNode = new CLLNode<T>( pData );

		// place the node in the list
		if ( m_pHead == NULL )
		{
			// list is empty
			m_pHead = pNewNode;
			m_pHead->SetNext( NULL );
			m_pHead->SetLast( NULL );
		} // end if -- empty list
		else
		{
			pNewNode->SetNext( m_pHead );
			m_pHead->SetLast( pNewNode );
			m_pHead = pNewNode;
		} // end else

		return pData;
	} // end queue

	// place at the end of the linked list
	T* QueueEnd( T* pData )
	{
		CLLNode<T>* pNewNode;

		pNewNode = new CLLNode<T>( pData );

		// place the node in the list
		if ( m_pHead == NULL )
		{
			// list is empty
			m_pHead = pNewNode;
			m_pHead->SetNext( NULL );
			m_pHead->SetLast( NULL );
		} // end if -- empty list
		else
		{
			// find the end, and add it there
			CLLNode<T> *pNode;
			
			pNode = m_pHead;
			while ( pNode->Next() != NULL )
			{
				pNode = pNode->Next();
			} // end while

			pNode->SetNext( pNewNode );
			pNewNode->SetNext( NULL );
			pNewNode->SetLast( pNode );
		} // end else

		return pData;
	} // end queue

	void RemoveNoDelete( CLLNode<T>* pNode )
	{
		if ( pNode == m_pHead )
		{
			// IF THERE IS A SUCCESSOR
			if ( pNode->Next() != NULL )
			{
				pNode->Next()->SetLast( NULL ) ;
			}

			// IF THERE ARE ANY REMAINING ITEMS
			if ( pNode->Next() != NULL )
			{
				// MAKE THE NEXT ONE THE m_pHead
				m_pHead = pNode->Next();
				m_pHead->SetLast( NULL );
			}
			// OTHERWISE, NULL THE m_pHead
			else
			{
				m_pHead = NULL;
			}

			DELETE_SAFELY( pNode );
			pNode = NULL;
		}
		// OTHERWISE, IF IT IS SOMEWHERE ELSE IN THE LIST
		else
		{
			// LINK THE PREVIOUS ITEM TO THE FOLLOWING ITEM
			if ( pNode->Last() != NULL )
			{
				pNode->Last()->SetNext( pNode->Next() );
			}

			// AND LINK THE FOLLOWING ITEM TO THE PREVIOUS ONE
			if ( pNode->Next() != NULL )
			{
				pNode->Next()->SetLast( pNode->Last() );
			}

			DELETE_SAFELY( pNode );
			pNode = NULL;
		}
	} 

	void Remove( CLLNode<T>* pNode )
	{
		T*	pData;

		if ( pNode == m_pHead )
		{
			// IF THERE IS A SUCCESSOR
			if ( pNode->Next() != NULL )
			{
				pNode->Next()->SetLast( NULL ) ;
			}

			// IF THERE ARE ANY REMAINING ITEMS
			if ( pNode->Next() != NULL )
			{
				// MAKE THE NEXT ONE THE m_pHead
				m_pHead = pNode->Next();
				m_pHead->SetLast( NULL );
			}
			// OTHERWISE, NULL THE m_pHead
			else
			{
				m_pHead = NULL;
			}

			pData = pNode->Data();
			DELETE_SAFELY( pData );
			DELETE_SAFELY( pNode );
			pData = NULL;
			pNode = NULL;
		}
		// OTHERWISE, IF IT IS SOMEWHERE ELSE IN THE LIST
		else
		{
			// LINK THE PREVIOUS ITEM TO THE FOLLOWING ITEM
			if ( pNode->Last() != NULL )
			{
				pNode->Last()->SetNext( pNode->Next() );
			}

			// AND LINK THE FOLLOWING ITEM TO THE PREVIOUS ONE
			if ( pNode->Next() != NULL )
			{
				pNode->Next()->SetLast( pNode->Last() );
			}

			pData = pNode->Data();
			DELETE_SAFELY( pData );
			DELETE_SAFELY( pNode );
			pData = NULL;
			pNode = NULL;
		}
	} // end destroy

	// end destroy

	// deletes data stored in the list as the list is cleaned
	void Destroy( void )
	{
		T*		pData;
		CLLNode<T>	*pNode, *pTempNode;

		pNode = m_pHead;
		while ( pNode != NULL )
		{
			pTempNode = pNode->Next();
			pNode->SetNext( NULL );
			pNode->SetLast( NULL );
			pData = pNode->Data();
			DELETE_SAFELY( pData );
			DELETE_SAFELY( pNode );
			pData = NULL;
			pNode = NULL;

			pNode = pTempNode;
		} // end while

		pTempNode = NULL;
		m_pHead = NULL;
	} // end destroy

	// does NOT delete data in the list as it is cleaned
	void Clear( void )
	{

		CLLNode<T> *pNode, * pTempNode;

		pNode = m_pHead;
		while ( pNode != NULL )
		{
			pTempNode = pNode->Next();
			pNode->SetNext( NULL );
			pNode->SetLast( NULL );
			DELETE_SAFELY( pNode );
			pNode = NULL;

			pNode = pTempNode;
		} // end while

		pTempNode = NULL;
		m_pHead = NULL;
	} // end destroy

	CLLNode<T>* GetFirst( void )
	{
		return m_pHead;
	}

private:
	// member functions for traversing the list
	T First( void )
	{
		if ( m_pHead == NULL )
		{
			return( NULL );
		}

		return( m_pHead->Data() );
	} // end first


	T*		Next( void );

	CLLNode<T>*		m_pHead;
}; // end CLList

#endif