/*
	bone.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#ifndef _BONE_H
#define _BONE_H

#include "d3dx8_shim.h"

#include <d3d8.h>
#include <d3dx8.h>
#include <string>


// bone maintains its transformation, name, and list of children
class CBoneNode
{
public :

			CBoneNode( const std::string& Name,				// name of bone
				   const D3DXMATRIX& Transformation ) :	// base xform
														m_Name( Name ),
														m_Transformation( Transformation ),
														m_DefaultTransformation( Transformation ),
														m_WorldspaceTransformation( Transformation ),
														m_pParent( NULL )
			{
			};

			~CBoneNode( void ) 
			{
				for( uint32 i = 0; i < m_pChildren.size(); i++ )
				{
					DELETE_SAFELY( m_pChildren[i] );
				}
				m_pChildren.clear();
			};

	void	AddChild( CBoneNode* pBone )	
			{	
				m_pChildren.push_back( pBone );	
				pBone->SetParent( this );
			};

	void	SetParent( CBoneNode* pBone )	{	m_pParent = pBone;				};

	bool	IsRoot( void )				{	return m_pParent == NULL;		};

	bool	HasChildren( void )			{	return m_pChildren.size() > 0;	};

	const std::string&	Name( void )	{	return m_Name;					};

	const std::vector< CBoneNode* >&	Children( void ) const	{	return m_pChildren;			};

	void	SetTransformation( const D3DXMATRIX& Transformation )	{	m_Transformation = Transformation;	};

	const	D3DXMATRIX& Transformation( void )				{	return m_Transformation;			};
	const	D3DXMATRIX& WorldspaceTransformation( void )	{	return m_WorldspaceTransformation;	};

	void	UpdateTransformation( const D3DXMATRIX& ParentTransformation )	// parent bone transformation
	{
		m_WorldspaceTransformation = m_Transformation * ParentTransformation;
		for( uint32 i = 0; i < m_pChildren.size(); i++ )
		{
			m_pChildren[i]->UpdateTransformation( m_WorldspaceTransformation );
		}
	}

	bool	BoneExists( const std::string& BoneName )	// name of bone to find
	{

		if( BoneName == m_Name )
		{
			return kTrue;
		}
		for( uint32 i = 0; i < m_pChildren.size(); i++ )
		{
			if( m_pChildren[i]->BoneExists( BoneName ) )
			{
				return kTrue;
			}
		}
		return kFalse;
	}

	CBoneNode*	Parent( void )		{	return m_pParent;			};

	CBoneNode*	GetBone( const std::string& BoneName )	// name of bone to find
	{

		if(  m_Name == BoneName )
		{
			return this;
		}
		for( uint32 i = 0; i < m_pChildren.size(); i++ )
		{
			CBoneNode* pBone = m_pChildren[i]->GetBone( BoneName );
			if( pBone != NULL )
			{
				return pBone;
			}
		}
		return NULL;
	}

private:
	D3DXMATRIX				m_Transformation;
	D3DXMATRIX				m_DefaultTransformation;
	D3DXMATRIX				m_WorldspaceTransformation;

	std::string				m_Name;

	CBoneNode*					m_pParent;

	std::vector< CBoneNode* >	m_pChildren;

}; // CBoneNode Class

#endif