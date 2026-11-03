//
// RenderCollection.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The RenderCollection class is an abstract base class designed to manage the device render state for a collection
//	of renderable objects.  This class allows a user to set a Pre-RenderState and a Post-RenderState.  The 
//	Pre-RenderState is set before any of the objects are rendered and the Post-RenderState is set after object 
//	rendering is completed.  In addition the user can elect to save the current render state (by setting 
//	ResetRenderState() to "true") before rendering objects and restoring that saved state after the objects have 
//	completed rendering.
//	Any combination of PreRenderState, PostRestoreState, ResetRenderState can be used for rendering.  However it 
//	doesn't make sense to define both PostRenderState and ResetRenderState.  In this case the PostRenderState will 
//	be the final render state after rendering.
//	To use this class a new class must be derived from this class that implements the RenderObjects() method.
//	The RenderObjects() method should be implemented to render a collection of objects that share the pre-render 
//	state.
//

#pragma once


#include <d3d9.h>
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/Graphics/IRenderable.h"


namespace WT
{
namespace Graphics
{

typedef System::SmartPointer<class CRenderCollection> HRenderCollection;
class CRenderCollection : public System::CObject, public CRenderableImpl
{
public:

	WT_BEGIN_ABSTRACT_CLASS( CRenderCollection, System::CObject );
		WT_INHERITS_INTERFACE( IRenderable )
	WT_END_CLASS();

	CRenderCollection() : m_pPreRenderState( NULL ), m_pPostRenderState( NULL ), m_ResetRenderState( false )
	{
	}
	virtual ~CRenderCollection()
	{
		ReleaseStates();
	}

	
	// Accessors

	virtual void SetPreRenderState( IDirect3DStateBlock9* pPreRenderState )
	{
		if ( pPreRenderState )
			pPreRenderState->AddRef();
		SafeRelease( m_pPreRenderState );
		m_pPreRenderState = pPreRenderState;
	}

	virtual IDirect3DStateBlock9* GetPreRenderState()
	{
		return m_pPreRenderState;
	}

	virtual void SetPostRenderState( IDirect3DStateBlock9* pPostRenderState )
	{
		if ( pPostRenderState )
			pPostRenderState->AddRef();
		SafeRelease( m_pPostRenderState );
		m_pPostRenderState = pPostRenderState;
	}

	virtual IDirect3DStateBlock9* GetPostRenderState()
	{
		return m_pPostRenderState;
	}

	virtual void ResetRenderState( bool ResetRenderState )
	{
		m_ResetRenderState = ResetRenderState;
	}


	virtual void ReleaseStates()
	{
		SafeRelease( m_pPreRenderState );
		SafeRelease( m_pPostRenderState );
	}

	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
	{
		if (pDevice == NULL)
		{
			assert(0);
			return;
		}

		if (!Context.ShouldRender( *this ))
		{
			return;
		}

		// Pre-render state set up
		IDirect3DStateBlock9* pResetRenderState = NULL;
		if ( m_ResetRenderState )
		{
			pDevice->CreateStateBlock( D3DSBT_ALL, &pResetRenderState );
		}
		if ( m_pPreRenderState != NULL )
		{
			m_pPreRenderState->Apply();
		}

		// Render collection objects
		RenderObjects( pDevice, Context );

		// Post-render state restore
		if ( pResetRenderState != NULL )
		{
			assert( m_pPostRenderState == NULL );	// Can only have one post-render state.  m_pPostRenderState wins.
			pResetRenderState->Apply();
			SafeRelease( pResetRenderState );
		}
		else if ( m_pPostRenderState != NULL )
		{
			m_pPostRenderState->Apply();
		}
	}

protected: // methods

	virtual void RenderObjects( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context ) = 0;

protected: // data

	IDirect3DStateBlock9*	m_pPreRenderState;
	IDirect3DStateBlock9*	m_pPostRenderState;
	bool					m_ResetRenderState : 1;
};


} // namespace Graphics
} // WT
