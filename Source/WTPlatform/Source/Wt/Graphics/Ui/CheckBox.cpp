//
// CheckBox.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/CheckBox.h"


namespace WT
{
namespace Graphics
{
namespace UI
{


bool CCheckBox::RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return true;
	}

	// Render the sprite image (if available), then overlay text caption
	if( m_pSprite != NULL )
	{
		m_pSprite->Render( pDevice, Context );
	
		// Render the sprite a second time using the UV coordinates for the 'checked' state if needed
		if( m_Checked )
		{
			// Remember previous UVs
			CVector2f const CachedTopLeftUV = m_pSprite->GetTopLeftUV( );
			CVector2f const CachedBottomRightUV = m_pSprite->GetBottomRightUV( );

			// Update sprite UVs for checked image
			m_pSprite->SetTopLeftUV( m_CheckedStartUV );
			m_pSprite->SetBottomRightUV( m_CheckedEndUV );

			m_pSprite->Render( pDevice, Context );

			// Restore previous UVs
			m_pSprite->SetTopLeftUV( CachedTopLeftUV );
			m_pSprite->SetBottomRightUV( CachedBottomRightUV );
		}
	}

	return CLabel::RenderInternal( pDevice, Context );
}

void CCheckBox::UpdateSprite( bool UpdateTexture, bool UpdateTextureUVs )
{
	// The base implementation will update the texture UVs for the sprite's 'normal' layer
	// Apply the same logic to the UVs for the 'checked' state as well
	CControl::UpdateSprite( UpdateTexture, UpdateTextureUVs );

	if( UpdateTextureUVs && m_pSprite != NULL )
	{
		int32 TexWidth, TexHeight;
		if ( !m_pTexMaterial->GetTextureSize( 0, TexWidth, TexHeight ) )
		{
			assert(false);
			return;
		}

		if( m_TextureUVsChecked.right != 0 && m_TextureUVsChecked.bottom != 0 && 
			TexWidth != 0 && TexHeight != 0 )	// Avoid divide-by-zero
		{
			float32 const OneDivWidth  = 1.0f / static_cast<float32>( TexWidth );
			float32 const OneDivHeight = 1.0f / static_cast<float32>( TexHeight );

			m_CheckedStartUV.x = static_cast<float32>( m_TextureUVsChecked.left ) * OneDivWidth;
			m_CheckedStartUV.y = static_cast<float32>( m_TextureUVsChecked.top )  * OneDivHeight;

			m_CheckedEndUV.x = static_cast<float32>( m_TextureUVsChecked.right )  * OneDivWidth;
			m_CheckedEndUV.y = static_cast<float32>( m_TextureUVsChecked.bottom ) * OneDivHeight;
		}
	}
}

void CCheckBox::UpdateLayout( )
{
	// Update bounds
	CControl::UpdateLayout( );

	// If the bounding box is still 0 in size, defer the layout until later
	if( m_BoundingBox.Width() == 0.0f || m_BoundingBox.Height() == 0.0f )
	{
		m_LayoutDirty = true;
		return;
	}

	CVector2f const AbsPosition = GetAbsolutePosition( );
	float32 const& X = AbsPosition.x;
	float32 const& Y = AbsPosition.y;

	// Update the sprite object to be justified at (X,Y) and be Height*Height in size
	// Then update the text object to be justified at (1.25*X,) and centered in the y-direction

	// TODO: Rotations are probably not handled correctly 

	float32 SpriteOffset = 0.0f;

	// Update sprite object
	if( m_pSprite != NULL )
	{
		SpriteOffset = m_pSprite->GetSize( ).x;

		m_pSprite->SetRotationCenter( m_CenterX, m_CenterY );
		m_pSprite->SetZRotation( m_RotationZ );

		// Set sprite position to upper-left of bounds
		m_pSprite->SetPosition( X, Y );

		// Make sprite size square (height*height)
		m_pSprite->SetSize( m_Height, m_Height );
	}

	// Update text object
	if( m_pText != NULL )
	{
		m_pText->SetRotationCenter( m_CenterX, m_CenterY );
		m_pText->SetRotationZ( m_RotationZ );

		CVector2f const Size = m_pText->GetSize( );

		// Calculate the y-offset for the centered text label
		float32 const YOffset = 0.5f * (m_Height - Size.y);

		// Position the text object 125% away from the left side of the sprite object in the x-direction,
		// and centered in the y-direction
		float32 const XPos = (float32)(int32)(X + (1.25f * SpriteOffset));
		m_pText->SetPosition( XPos, (float32)(int32)(Y + YOffset) );
	}
}


} // namespace UI
} // namespace Graphics
} // namespace WT
