//
// ControlBehaviors.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/Graphics/UI/Control.h"
#include "WT/Graphics/UI/ControlBehavior.h"
#include "WT/Graphics/UI/Label.h"
#include "WT/Math/Math.h"
#include "WT/System/Timer.h"
#include "WT/System/DateTime.h"
#include <d3d9.h>


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace Math;

class CControlTextureUVs : public CControlBehavior
{
public:

	WT_CLASS( CControlTextureUVs, CControlBehavior )

	CControlTextureUVs( )	
	{
	}
	CControlTextureUVs( const CControlTextureUVs& Other)
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlTextureUVs( )
	{
	}


	// Sets the texture coordinates associated with the control's base element layer
	virtual void SetTextureUVs( CRect2i TextureUVs )
	{
		m_TextureUVs = TextureUVs;
	}

	// Retrieves the texture coordinates associated with the control's base element layer
	virtual CRect2i GetTextureUVs( ) const
	{
		return m_TextureUVs;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl* pControl, IDirect3DDevice9* )
	{
		if( pControl != NULL )
		{
			pControl->SetTextureUVs( m_TextureUVs );
		}
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlTextureUVs& Other)
	{
		CControlBehavior::CopyPropertiesFrom( Other );
		m_TextureUVs = Other.m_TextureUVs;
	}

protected:	// data

	CRect2i	m_TextureUVs;
};


class CControlTextColor : public CControlBehavior
{
public:
	
	WT_CLASS( CControlTextColor, CControlBehavior )

	CControlTextColor( ) : m_TextColor( 0xff000000 )
	{
	}
	CControlTextColor( const CControlTextColor& Other )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlTextColor( )
	{
	}


	// Sets the text color associated with the control's base element layer under the given state
	virtual void SetTextColor( uint32 Color )
	{
		m_TextColor = Color;
	}

	// Retrieves the text color associated with the control's base element layer under the given state
	virtual uint32 GetTextColor( ) const
	{
		return m_TextColor;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl* pControl, IDirect3DDevice9* )
	{
		// TODO : Performance review - This is done inside the render loop.  Doing a DynamicCast here may be a
		// performance problem.  Think about making SetTextColor a method in the CControl base class, that defaults
		// to NOP.
		CLabel* pLabel = DynamicCast<CLabel>( pControl );
		if( pLabel != NULL )
		{			
			pLabel->SetTextColor( m_TextColor );
		}
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlTextColor& Other)
	{
		CControlBehavior::CopyPropertiesFrom( Other );
		m_TextColor = Other.m_TextColor;
	}

protected: // data

	uint32	m_TextColor;
};


class CControlTransform : public CControlBehavior
{
public:
	
	WT_CLASS( CControlTransform, CControlBehavior )

	CControlTransform( )	: m_AppliedOffsets( 0 )
	{
	}
	CControlTransform( const CControlTransform& Other )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlTransform( )
	{
	}

	// Sets the size offset in pixels
	virtual void SetSizeOffset( const CVector2f& SizeOffset )
	{
		m_SizeOffset = SizeOffset;
		if( m_SizeOffset.x != 0.0f && m_SizeOffset.y != 0.0f )
		{
			m_AppliedOffsets |= ESizeOffset;
		}
		else
		{
			m_AppliedOffsets &= ~ESizeOffset;
		}
	}

	// Retrieves the size offset in pixels
	virtual CVector2f GetSizeOffset( ) const
	{
		return m_SizeOffset;
	}

	// Sets the position offset in pixels
	virtual void SetPositionOffset( const CVector2f& PositionOffset )
	{
		m_PositionOffset = PositionOffset;
		if( m_PositionOffset.x != 0.0f && m_PositionOffset.y != 0.0f )
		{
			m_AppliedOffsets |= EPositionOffset;
		}
		else
		{
			m_AppliedOffsets &= ~EPositionOffset;
		}
	}

	// Retrieves the position offset in pixels
	virtual CVector2f GetPositionOffset() const
	{
		return m_PositionOffset;
	}

	// Sets the offset of the rotation center in pixels
	virtual void SetCenterRotationOffset( const CVector2f& CenterRotationOffset )
	{
		m_CenterRotationOffset = CenterRotationOffset;
		if( m_CenterRotationOffset.x != 0.0f && m_CenterRotationOffset.y != 0.0f )
		{
			m_AppliedOffsets |= ECenterRotationOffset;
		}
		else
		{
			m_AppliedOffsets &= ~ECenterRotationOffset;
		}
	}

	// Retrieves the offset of the rotation center in pixels
	virtual CVector2f GetCenterRotationOffset( ) const
	{
		return m_CenterRotationOffset;	
	}

	// Sets the z-axis rotation offset, in radians
	virtual void SetZRotationOffset( float32 ZRotationOffset )
	{
		m_ZRotationOffset = ZRotationOffset;
		if( m_ZRotationOffset != 0.0f )
		{
			m_AppliedOffsets |= EZRotationOffset;
		}
		else
		{
			m_AppliedOffsets &= ~EZRotationOffset;
		}
	}

	// Retrieves the z-axis rotation offset, in radians
	virtual float32 GetZRotationOffset( ) const
	{
		return m_ZRotationOffset;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl* pControl, IDirect3DDevice9* );

	virtual void RestoreControlInternal( CControl* pControl, IDirect3DDevice9* );

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlTransform& Other );

protected: // types

	// Describes various offsets that can be applied
	enum 
	{
		ESizeOffset				= 1<<0,
		EPositionOffset			= 1<<1,
		ECenterRotationOffset	= 1<<2,
		EZRotationOffset		= 1<<3,
	};

protected: // data

	// User properties
	CVector2f	m_SizeOffset;
	CVector2f	m_PositionOffset;
	CVector2f	m_CenterRotationOffset;
	float32		m_ZRotationOffset;

	// Internal properties
	CVector2f	m_RestoreSize;
	CVector2f	m_RestorePosition;
	CVector2f	m_RestoreCenterRotation;
	float32		m_RestoreZRotation;
	uint32		m_AppliedOffsets;			// records which user properties to apply
};


class CControlEffect : public CControlBehavior
{
public:
	
	WT_CLASS( CControlEffect, CControlBehavior )

	CControlEffect( ) : m_pEffect( NULL ), m_pRestoreEffect( NULL )
	{
	}
	CControlEffect( const CControlEffect& Other) : m_pRestoreEffect( NULL )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlEffect( )
	{
		SafeRelease( m_pEffect );
	}


	// Sets the high-level effect object used by the behavior
	virtual void SetEffect( ID3DXEffect* pEffect )
	{
		if ( pEffect )
			pEffect->AddRef();

		SafeRelease( m_pEffect );
		m_pEffect = pEffect;
	}

	// Retrieves the high-level effect object used by the behavior
	virtual ID3DXEffect* GetEffect( ) const
	{
		return m_pEffect;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl* pControl, IDirect3DDevice9* )
	{
		m_pRestoreEffect = pControl->GetEffect( );
		pControl->SetEffect( m_pEffect );
	}

	virtual void RestoreControlInternal( CControl* pControl, IDirect3DDevice9* )
	{
		// CControl::SetEffect() will AddRef the D3DEffect object so we need to reduce the RefCount by
		// one here.
		if ( m_pRestoreEffect ) m_pRestoreEffect->Release();
		pControl->SetEffect( m_pRestoreEffect );
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlEffect& Other )
	{
		CControlBehavior::CopyPropertiesFrom( Other );

		SetEffect( Other.m_pEffect );
	}

protected: // data

	// User properties
	ID3DXEffect*	m_pEffect;

	// Internal properties
	ID3DXEffect*	m_pRestoreEffect;
};


class CControlVisibility : public CControlBehavior
{
public:
	
	WT_CLASS( CControlVisibility, CControlBehavior )


	CControlVisibility( )	
		: m_Timeout( 0 )
		, m_MakeVisible( true )
		, m_pTimer( NULL )
	{
		m_pTimer = new WT::System::CTimer( System::ETimerResolution::Medium );
	}
	CControlVisibility( const CControlVisibility& Other ) : m_pTimer( NULL )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlVisibility( )
	{
		SafeDelete( m_pTimer );
	}


	virtual void SetTimeout( CTimeSpan Timeout )
	{
		m_Timeout = Timeout;
		ResetTimer( );
	}

	virtual CTimeSpan GetTimeout( ) const
	{
		return m_Timeout;
	}

	// Sets whether the controller makes the control visible or invisible after the 
	// timeout period has expired
	virtual void SetMakeVisible( bool MakeVisible )
	{
		m_MakeVisible = MakeVisible;
	}

	// Determines whether the controller makes the control visible or invisible after the 
	// timeout period has expired
	virtual bool GetMakeVisible( ) const
	{
		return m_MakeVisible;
	}

	// Resets the internal controller timer 
	virtual void ResetTimer( )
	{
		if( m_pTimer )
		{
			m_pTimer->Start( );
		}
	}

protected:	// methods

	virtual void UpdateControlInternal( CControl* pControl, IDirect3DDevice9* )
	{
		if( pControl != NULL && m_pTimer != NULL )
		{
			if( m_pTimer->GetTotalTime( ) >= m_Timeout.TotalMilliseconds( ) )
			{
				pControl->SetVisible( m_MakeVisible );
				ResetTimer( );
			}
		}
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlVisibility& Other )
	{
		CControlBehavior::CopyPropertiesFrom( Other );

		m_Timeout = Other.m_Timeout;
		m_MakeVisible = Other.m_MakeVisible;
		m_pTimer = new CTimer( System::ETimerResolution::Medium );
		ResetTimer( );
	}

protected:	// data

	// User properties
	System::CTimeSpan	m_Timeout;
	bool				m_MakeVisible;

	// Internal properties
	System::CTimer*		m_pTimer;
};


class CControlRenderState : public CControlBehavior
{
public:
	
	WT_CLASS( CControlRenderState, CControlBehavior )


	CControlRenderState( ) : m_RenderStateValue( 0 ), m_RenderStateRestoreValue( 0 )
	{
	}
	CControlRenderState( const CControlRenderState& Other )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlRenderState( )
	{
	}

	// TODO : Performance review - Consider using D3D State Blocks.

	// Sets the render state influenced by this controller
	virtual void SetRenderState( D3DRENDERSTATETYPE RenderState )
	{
		m_RenderState = RenderState;
	}

	// Retrieves the render state influenced by this controller
	virtual D3DRENDERSTATETYPE GetRenderState( ) const
	{
		return m_RenderState;
	}

	// Sets a single render state parameter 
	virtual void SetRenderStateValue( uint32 Value )
	{
		m_RenderStateValue = Value;
	}

	// Retrieves the value of the given render state parameter 
	virtual uint32 GetRenderStateValue( ) const
	{
		return m_RenderStateValue;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl*, IDirect3DDevice9* pDevice )
	{
		if( pDevice != NULL )
		{
			// TODO : Review - GetRenderState will not work on Pure HW Devices
			pDevice->GetRenderState( m_RenderState, &m_RenderStateRestoreValue );
			pDevice->SetRenderState( m_RenderState, m_RenderStateValue );
		}
	}

	// Restores the given control to its previous state 
	// This method will typically be called by controls after rendering is performed
	virtual void RestoreControlInternal( CControl*, IDirect3DDevice9* pDevice )
	{
		if( pDevice != NULL )
		{
			pDevice->SetRenderState( m_RenderState, m_RenderStateRestoreValue );
		}
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlRenderState& Other )
	{
		CControlBehavior::CopyPropertiesFrom( Other );
		m_RenderState = Other.m_RenderState;
		m_RenderStateValue = 0;
		m_RenderStateRestoreValue = 0;
	}

protected: // data

	D3DRENDERSTATETYPE	m_RenderState;
	uint32				m_RenderStateValue;

	uint32				m_RenderStateRestoreValue;
};


class CControlSamplerState : public CControlBehavior
{
public:

	WT_CLASS( CControlSamplerState, CControlBehavior )

	CControlSamplerState( )
	{
		m_StageIndex = 0;
		m_SamplerStateValue = 0;
		m_SamplerStateRestoreValue = 0;
	}
	CControlSamplerState( const CControlSamplerState& Other )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlSamplerState( )
	{
	}

	// TODO : Performance review - Consider using D3D State Blocks.

	// Sets the stage index influenced by this controller
	// The default value for this parameter is 0
	virtual void SetStageIndex( uint32 StageIndex )
	{
		m_StageIndex = StageIndex;
	}

	// Retrieves the stage index influenced by this controller
	virtual uint32 GetStageIndex( ) const
	{
		return m_StageIndex;
	}

	// Sets the stage state influenced by this controller
	virtual void SetSamplerState( D3DSAMPLERSTATETYPE SamplerState )
	{
		m_SamplerState = SamplerState;
	}

	// Retrieves the stage state influenced by this controller
	virtual D3DSAMPLERSTATETYPE GetSamplerState( ) const
	{
		return m_SamplerState;
	}

	// Sets a single sampler state parameter 
	virtual void SetSamplerStateValue( uint32 Value )
	{
		m_SamplerStateValue = Value;
	}

	// Retrieves the value of a single sampler state parameter 
	virtual uint32 GetSamplerStateValue( ) const
	{
		return m_SamplerStateValue;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl*, IDirect3DDevice9* pDevice )
	{
		if( pDevice != NULL )
		{
			pDevice->GetSamplerState( m_StageIndex, m_SamplerState, &m_SamplerStateRestoreValue );
			pDevice->SetSamplerState( m_StageIndex, m_SamplerState, m_SamplerStateValue );
		}
	}

	// Restores the given control to its previous state 
	// This method will typically be called by controls after rendering is performed
	virtual void RestoreControlInternal( CControl*, IDirect3DDevice9* pDevice )
	{
		if( pDevice != NULL )
		{
			pDevice->SetSamplerState( m_StageIndex, m_SamplerState, m_SamplerStateRestoreValue );
		}
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlSamplerState& Other )
	{
		CControlBehavior::CopyPropertiesFrom( Other );

		m_SamplerState = Other.m_SamplerState;
		m_StageIndex = Other.m_StageIndex;
		m_SamplerStateValue = 0;
		m_SamplerStateRestoreValue = 0;
	}

protected: // data

	D3DSAMPLERSTATETYPE	m_SamplerState;
	uint32				m_StageIndex;
	uint32				m_SamplerStateValue;

	uint32				m_SamplerStateRestoreValue;
};


class CControlTextureStageState : public CControlBehavior
{
public:

	WT_CLASS( CControlTextureStageState, CControlBehavior )

	CControlTextureStageState( )
	{
		m_StageIndex = 0;
		m_TextureStageStateValue = 0;
		m_TextureStageStateRestoreValue = 0;
	}
	CControlTextureStageState( const CControlTextureStageState& Other )
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlTextureStageState( )
	{
	}


	// Sets the stage index influenced by this controller
	// The default value for this parameter is 0
	virtual void SetStageIndex( uint32 StageIndex )
	{
		m_StageIndex = StageIndex;
	}

	// Retrieves the stage index influenced by this controller
	virtual uint32 GetStageIndex( ) const
	{
		return m_StageIndex;
	}

	// Sets the stage state influenced by this controller
	virtual void SetTextureStageState( D3DTEXTURESTAGESTATETYPE TextureStageState )
	{
		m_TextureStageState = TextureStageState;
	}

	// Retrieves the stage state influenced by this controller
	virtual D3DTEXTURESTAGESTATETYPE GetTextureStageState( ) const
	{
		return m_TextureStageState;
	}

	// Sets the state value for the currently assigned texture
	virtual void SetTextureStageStateValue( uint32 Value )
	{
		m_TextureStageStateValue = Value;
	}

	// Retrieves the state value for the currently assigned texture
	virtual uint32 GetTextureStageStateValue( ) const
	{
		return m_TextureStageStateValue;
	}

protected: // methods

	virtual void UpdateControlInternal( CControl*, IDirect3DDevice9* pDevice )
	{
		if( pDevice != NULL )
		{
			pDevice->GetTextureStageState( m_StageIndex, m_TextureStageState, &m_TextureStageStateRestoreValue );
			pDevice->SetTextureStageState( m_StageIndex, m_TextureStageState, m_TextureStageStateValue );
		}
	}

	// Restores the given control to its previous state 
	// This method will typically be called by controls after rendering is performed
	virtual void RestoreControlInternal( CControl*, IDirect3DDevice9* pDevice )
	{
		if( pDevice != NULL )
		{
			pDevice->SetTextureStageState( m_StageIndex, m_TextureStageState, m_TextureStageStateRestoreValue );
		}
	}

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlTextureStageState& Other )
	{
		CControlBehavior::CopyPropertiesFrom( Other );

		m_TextureStageState = Other.m_TextureStageState;
		m_StageIndex = Other.m_StageIndex;
		m_TextureStageStateValue = 0;
		m_TextureStageStateRestoreValue = 0;
	}

protected: // data

	D3DTEXTURESTAGESTATETYPE	m_TextureStageState;
	uint32						m_StageIndex;
	uint32						m_TextureStageStateValue;

	uint32						m_TextureStageStateRestoreValue;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
