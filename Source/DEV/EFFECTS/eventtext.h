/*
	eventtext.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _EVENTTEXT_H
#define _EVENTTEXT_H


#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../GAMECLIENT/gamerules.h"

#include "../UI/text.h"
#include "../UI/fontmetric.h"

#include "../CHARACTER/character.h"

// event text that rises and fades out

class CEventText
{

public:

	CEventText( CCharacter* pParent,			// parent character
				CCharacter* pTarget,			// target character
				const D3DXVECTOR3& Position,	// worldspace position
				const std::string& Text,		// text to appear
				float32 Scale,					// scale
				bool Bolded, 					// text bolded?
				float32 FadeRate,				// fade rate
				bool SecondaryFont = kFalse ) :			// secondary font use?
				m_Position( Position ),
				m_Text( Text ),
				m_Scale( Scale ),
				m_FadeRate( FadeRate ),
				m_Bolded( Bolded ),
				m_SecondaryFont( SecondaryFont ),
				m_pText( NULL ),
				m_pParent( pParent ),
				m_pTarget( pTarget ),
			    m_Opacity( 255 )
	{
	}


	~CEventText( void )
	{
		DELETE_SAFELY( m_pText );
	}

	void Update( float32 TimeElapsed )	// time elapsed in seconds
	{
		m_Opacity -= TimeElapsed * 300 * m_FadeRate;
		if( m_Opacity <= 0 )
		{
			m_Opacity = 0;
		}
		m_Position.y += 2.0f * TimeElapsed;
	}
	
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	CreateText( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device)
						CFontMetric* pFontMetric,				// font metric
						CMaterial* pFontMaterial )				// font material
	{
		m_pText = new CText( pD3DDevice,
							 pFontMaterial,
							 pFontMetric,
							 m_Text,
							 0,
							 0,
							 m_Scale,
							 m_Bolded,
							 KJustifyCenter );
	}

						
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	CCharacter*	Parent( void )				{	return m_pParent;		};
	CCharacter*	Target( void )				{	return m_pTarget;		};

	bool		InFrustum( CFrustum& pFrustum )			// frustum to cull to
	{
		// if we're not in the frustum, don't render at all
		if( !pFrustum.SphereIntersects( m_Position, 1 ) )
		{
			return kFalse;
		}

		return kTrue;
	} 

	bool				SecondaryFont( void )	{	return m_SecondaryFont;		};

	CText*				Text( void )		{	return m_pText;			};

	const D3DXVECTOR3&	Position( void )	{	return m_Position;		};

	float32				Opacity( void )		{	return m_Opacity;		};

private:

	float32				m_FadeRate;

	D3DXVECTOR3			m_Position;
	std::string			m_Text;
	bool				m_Bolded;
	bool				m_SecondaryFont;

	float32				m_Scale;
	float32				m_Opacity;

	CText*				m_pText;

	CCharacter*			m_pParent;
	CCharacter*			m_pTarget;
};
#endif