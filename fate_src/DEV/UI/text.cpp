/*
	text.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004

	CHANGE HISTORY

	4/16/2004 - Added support for carriage returns - reworked justification code also,
				now much cleaner

	5/24/2004 - made the update method recognize changes to parameters, and only recreate when those
				parameters are different

	5/25/2004 - Added methods for positioning, rescaling, and otherwise altering a text object
				without having to call update again

	5/26/2004 - Was a bug in setposition code that was using worldspace instead of screenspace when
				updating, which resulted in bad behavior with settext and setjustification/setbold
	
*/
#include <D3D8.h>
#include <d3dx8.h>

#include <string>
#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/math3d.h"
#include "../MEDIA/material.h"
#include "text.h"
#include "fontmetric.h"

CText::CText( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
			  CMaterial* pMaterial,					// material of the font texture we want to use
			  CFontMetric* pFontMetric,				// font metric describing character widths/positioning
			  const std::string& Text,			// text to display
			  float32 X,							// x location onscreen (pixel coords from top left)
			  float32 Y,							// y location onscreen (pixel coords from top left)
			  float32 Scale,						// scale factor for the text (1.0f)
			  bool Bold,							// bolded?
			  EJustification Justification,			// justification for the text
			  uint32 ScreenWidth,					// screen width used in orthographic projection
			  uint32 ScreenHeight ) :				// screen height used in orthographic projection
										m_ScreenWidth( ScreenWidth ),
										m_ScreenHeight( ScreenHeight ),
										m_pVB( NULL ),
										m_pIB( NULL ),
										m_BufferExists( kFalse ),
										m_Wrapped( kFalse ),
										m_Wrapping( kFalse ),
										m_Scale( Scale ),
										m_Bold( Bold ),
										m_Justification( Justification ),
										m_TextCharacters( -1 ),
										m_LastLength( -1 ),
										m_pMaterial( pMaterial ),
										m_pMaterial2( pMaterial ),
										m_pFontMetric( pFontMetric ),
										m_LastText( "" ),
										m_ScreenX(0.0f),
										m_ScreenY(0.0f),
										m_X(0.0f),
										m_Y(0.0f),
										m_WrapWidth(0.0f)
{
	ZeroMemory ( &m_MinBounds, sizeof ( m_MinBounds ));
	ZeroMemory ( &m_MaxBounds, sizeof ( m_MaxBounds ));

	// immediately create the text
	Update( pD3DDevice,
			Text,
			X,
			Y,
			Scale,
			Bold,
			Justification,
			ScreenWidth,
			ScreenHeight );
} // CText::CText()

CText::CText( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
			  CMaterial* pMaterial,					// material of the font texture we want to use
			  CMaterial* pMaterial2,				// material of the bold font texture we want to use
			  CFontMetric* pFontMetric,				// font metric describing character widths/positioning
			  const std::string& Text,			// text to display
			  float32 X,							// x location onscreen (pixel coords from top left)
			  float32 Y,							// y location onscreen (pixel coords from top left)
			  float32 Scale,						// scale factor for the text (1.0f)
			  bool Bold,							// bolded?
			  EJustification Justification,			// justification for the text
			  uint32 ScreenWidth,					// screen width used in orthographic projection
			  uint32 ScreenHeight ) :				// screen height used in orthographic projection
										m_ScreenWidth( ScreenWidth ),
										m_ScreenHeight( ScreenHeight ),
										m_pVB( NULL ),
										m_pIB( NULL ),
										m_BufferExists( kFalse ),
										m_Wrapped( kFalse ),
										m_Wrapping( kFalse ),
										m_Scale( Scale ),
										m_Bold( Bold ),
										m_Justification( Justification ),
										m_TextCharacters( -1 ),
										m_LastLength( -1 ),
										m_pMaterial( pMaterial ),
										m_pMaterial2( pMaterial2 ),
										m_pFontMetric( pFontMetric ),
										m_LastText( "" ),
										m_ScreenX(0.0f),
										m_ScreenY(0.0f),
										m_X(0.0f),
										m_Y(0.0f),
										m_WrapWidth(0.0f)
{
	ZeroMemory ( &m_MinBounds, sizeof ( m_MinBounds ));
	ZeroMemory ( &m_MaxBounds, sizeof ( m_MaxBounds ));

	// immediately create the text
	Update( pD3DDevice,
			Text,
			X,
			Y,
			Scale,
			Bold,
			Justification,
			ScreenWidth,
			ScreenHeight );
} // CText::CText()

CText::~CText()
{
	m_pMaterial = NULL;
	m_pMaterial2 = NULL;
	m_pFontMetric = NULL;

	RELEASE_SAFELY( m_pVB );
	RELEASE_SAFELY( m_pIB );

} // CText::~CText()

void CText::SetPosition ( float32 X,							// x location onscreen (pixel coords from top left)
						  float32 Y,							// y location onscreen (pixel coords from top left)
						  uint32 ScreenWidth,					// screen width used in orthographic projection
						  uint32 ScreenHeight )					// screen height used in orthographic projection
{
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;
	m_ScreenX = X;
	m_ScreenY = Y;

	// calculate screenspace location of text
	X -= ScreenWidth/2;
	Y -= ScreenHeight/2;
	Y *= -1;

	uint32		TextureWidth	= m_pMaterial->Width();
	// how many pixels per character
	float32		CharacterWidth	= ( float32 ) TextureWidth / 16.0f;

	// we want it to be centered vertically in the text
	float32	YOffset	= -CharacterWidth * m_Scale * .5f;

	Y += YOffset;

	// actual position of the resulting mesh
	m_X = X;
	m_Y = Y;

} // CText::SetPosition ()						  

void CText::SetScale( LPDIRECT3DDEVICE8 pD3DDevice, // our direct3d device
					  float32 Scale )				// scale factor for the text (1.0f)
{
	Update( pD3DDevice, 
			m_LastText,
			m_ScreenX,
			m_ScreenY,
			Scale,
			m_Bold,
			m_Justification,
			m_ScreenWidth,
			m_ScreenHeight );
} // CText::SetScale()

void CText::SetJustification( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
							  EJustification Justification )	// justification for the text
{
	Update( pD3DDevice, 
			m_LastText,
			m_ScreenX,
			m_ScreenY,
			m_Scale,
			m_Bold,
			Justification,
			m_ScreenWidth,
			m_ScreenHeight );
} // CText::SetScale()

void CText::SetText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					 const std::string& Text )			// text to display
{
	Update( pD3DDevice, 
			Text,
			m_ScreenX,
			m_ScreenY,
			m_Scale,
			m_Bold,
			m_Justification,
			m_ScreenWidth,
			m_ScreenHeight );
} // CText::SetScale()

void CText::EnableWrapping( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
							float32 WrapWidth )				// pixels to wrap
{
	m_Wrapping = kTrue;
	m_WrapWidth = WrapWidth;

	Update( pD3DDevice, 
			m_LastText,
			m_ScreenX,
			m_ScreenY,
			m_Scale,
			m_Bold,
			m_Justification,
			m_ScreenWidth,
			m_ScreenHeight );
} // CText::EnableWrapping()

void CText::DisableWrapping( LPDIRECT3DDEVICE8 pD3DDevice )	// our direct3d device
{
	m_Wrapping = kFalse;

	Update( pD3DDevice, 
			m_LastText,
			m_ScreenX,
			m_ScreenY,
			m_Scale,
			m_Bold,
			m_Justification,
			m_ScreenWidth,
			m_ScreenHeight );
} // CText::EnableWrapping()

void CText::SetBold( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					 bool Bold )						// bold state to set
{
	Update( pD3DDevice, 
			m_LastText,
			m_ScreenX,
			m_ScreenY,
			m_Scale,
			Bold,
			m_Justification,
			m_ScreenWidth,
			m_ScreenHeight );
} // CText::SetBold()

void CText::Update( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					CMaterial* pMaterial,				// material of the font texture we want to use
					CFontMetric* pFontMetric,			// font metric describing character widths/positioning
					const std::string& Text,			// text to display
					float32 X,							// x location onscreen (pixel coords from top left)
					float32 Y,							// y location onscreen (pixel coords from top left)
					float32 Scale,						// scale factor for the text (1.0f)
					bool Bold,							// bolded?
					EJustification Justification,		// justification for the text
					uint32 ScreenWidth,					// screen width used in orthographic projection
					uint32 ScreenHeight )				// screen height used in orthographic projection
{
	m_pMaterial		= pMaterial;
	m_pFontMetric	= pFontMetric;

	Update( pD3DDevice,
			Text,
			X,
			Y,
			Scale,
			Bold,
			Justification,
			ScreenWidth,
			ScreenHeight );
}
void CText::Update( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					CMaterial* pMaterial,				// material of the font texture we want to use
					CMaterial* pMaterial2,				// another font texture for accomodating multiple sheets
					CFontMetric* pFontMetric,			// font metric describing character widths/positioning
					const std::string& Text,			// text to display
					float32 X,							// x location onscreen (pixel coords from top left)
					float32 Y,							// y location onscreen (pixel coords from top left)
					float32 Scale,						// scale factor for the text (1.0f)
					bool Bold,							// bolded?
					EJustification Justification,		// justification for the text
					uint32 ScreenWidth,					// screen width used in orthographic projection
					uint32 ScreenHeight )				// screen height used in orthographic projection
{
	m_pMaterial		= pMaterial;
	m_pMaterial2	= pMaterial2;
	m_pFontMetric	= pFontMetric;

	Update( pD3DDevice,
			Text,
			X,
			Y,
			Scale,
			Bold,
			Justification,
			ScreenWidth,
			ScreenHeight );
}
std::string CText::WrapText( std::string Message,	// message to wrap
							 float32 Scale,				// scale of font
							 float32 MaxPixels )			// pixel width to wrap to
{
	std::string FinalText;
	std::string TokenText;
	std::string SubTokenText;
	queue<std::string>	Tokens;
	queue<std::string>	SubTokens;

	TokenizeString( &Tokens, Message, " ", '#' );
	float32 LineWidth( 0 );
	uint32 TokensDone( 0 );
	while( Tokens.size() > 0 )
	{
		TokenText = Tokens.front();

		while( TokenText.find( "\n" ) != -1 )
		{
			int32 Index = TokenText.find( "\n" );
			TokenText.replace( Index, 1, " \r " );
		}

		while( TokenText.find( "\r" ) != -1 )
		{
			int32 Index = TokenText.find( "\r" );
			TokenText.replace( Index, 1, "\n" );
		}

		TokenizeString( &SubTokens, TokenText, " ", '#' );

		while( SubTokens.size() > 0 )
		{
			SubTokenText = SubTokens.front();
			bool CarriageReturn( kFalse );

			uint32 Chars = SubTokenText.length();
			float32 Pixels( 0 );

			for ( uint32 i = 0; i < Chars; i++ )
			{
				int32	Index	= SubTokenText[i];
				if( Index == 10 )
				{
					CarriageReturn = kTrue;
				}

				if( Index != 8 &&
					Index != 10 )
				{
					Index -= 32;
					// bold characters are on the bottom half ot the texture sheet
					if ( m_Bold )
					{
						//Index += 128;
					}
					Pixels += ( (float32)m_pFontMetric->CharacterWidth( Index ) + m_pFontMetric->Kerning() ) * Scale;
				}
			}	

			if( TokensDone != 0 && !CarriageReturn )
			{
				// space pixels
				Pixels += ( (float32)m_pFontMetric->CharacterWidth( 0 ) + m_pFontMetric->Kerning() ) * Scale;
			}

			SubTokens.pop();

			if( CarriageReturn )
			{
				LineWidth = 0;
				FinalText = FinalText + "\n";
			}
			else if( Pixels + LineWidth <= MaxPixels && !CarriageReturn )
			{
				LineWidth += Pixels;

				FinalText = FinalText + SubTokenText;
			}
			else
			{
				//we wrap
				if( TokensDone != 0 && !CarriageReturn )
				{
					FinalText = FinalText + "\n" + SubTokenText;
					LineWidth = Pixels;
				}
			}
		}

		Tokens.pop();
		if( Tokens.size() > 0 )
		{
			FinalText = FinalText + " ";
			LineWidth += ( m_pFontMetric->CharacterWidth( 0 ) + m_pFontMetric->Kerning() ) * Scale;
		}

		TokensDone++;
	}
	return FinalText;
} // CText::WrapText()

void CText::Update( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					const std::string& Text,			// text to display
					float32 X,							// x location onscreen (pixel coords from top left)
					float32 Y,							// y location onscreen (pixel coords from top left)
					float32 Scale,						// scale factor for the text (1.0f)
					bool Bold,							// bolded?
					EJustification Justification,		// justification for the text
					uint32 ScreenWidth,					// screen width used in orthographic projection
					uint32 ScreenHeight )				// screen height used in orthographic projection
{
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;
	// if we have no polys, or the size of the text has changed, we have to
	// recreate the Vert and Index buffers
	bool CreateNewBuffers = kTrue;
	HRESULT	hr;
	
	SetPosition( X, Y, ScreenWidth, ScreenHeight );

	// if the text is blank, do nothing
	if ( Text.length() < 1 )
	{
		m_TextCharacters = 0;
		m_LastText = Text;
		return;
	}

	// we only want to lock/unlock the mesh if ABSOLUTELY neccessary!
	if( Text == m_LastText && 
		m_Scale == Scale &&
		m_Justification == Justification &&
		m_Bold == Bold &&
		m_Wrapped == m_Wrapping &&
		m_pVB != NULL )
	{
		//m_TextCharacters = Text.length();
		return;
	}

	m_Wrapped = m_Wrapping;

	// store these as our new settings
	m_Scale = Scale;
	m_Justification = Justification;
	m_Bold = Bold;

	uint32		TextureWidth	= m_pMaterial->Width();
	// how many pixels per character
	float32		CharacterWidth	= ( float32 ) TextureWidth / 16.0f;
	// UV/Pixel ratio
	float32		PixelStep		= 1.0f / ( float32 ) TextureWidth;
	
	float32		CharacterHeight	= ( float32 ) TextureWidth / 16.0f;
	if( m_pFontMetric->CharacterHeight() != -1 )
	{
		CharacterHeight = m_pFontMetric->CharacterHeight();
	}


	m_LastText = Text;

	std::string FinalText = Text;
	if( m_Wrapping )
	{
		FinalText = WrapText( FinalText, m_Scale, m_WrapWidth );
	}

	m_TextCharacters = FinalText.length();


	if( m_TextCharacters == m_LastLength &&
		m_BufferExists )
	{
		CreateNewBuffers = kFalse;
	}

	// if text is the same length as last time,
	// we don't have to make new buffers - just lock the VB and move it around
	m_LastLength = FinalText.length();

	// if we currently have any existing geometry, clear it out,
	// but only if we need to make new buffers
	if ( m_BufferExists && CreateNewBuffers )
	{
		RELEASE_SAFELY( m_pVB );
		RELEASE_SAFELY( m_pIB );
	}

	DWORD	Vertices	= m_TextCharacters*4;
	DWORD	Indices		= m_TextCharacters*6;

	if( CreateNewBuffers )
	{
		hr = pD3DDevice->CreateVertexBuffer( Vertices * sizeof( PLAINVERTEXSCENE ),
											D3DUSAGE_WRITEONLY,
											D3DFVF_PLAINVERTEXSCENE,
											D3DPOOL_MANAGED,
											&m_pVB );
		// couldn't create a vert buffer - we failed
		if ( FAILED( hr ) )
		{
			m_BufferExists = kFalse;
			return;
		}
		m_BufferExists = kTrue;
	}


	std::vector < float32 >	LineWidth;
	float32	StartingPosition	= 0;
	// calculate the width of the mesh

	for ( int32 i = 0; i < m_TextCharacters; i++ )
	{
		//int32	Index	= FinalText[i] - 32;
		int32	Index	= FinalText[i]; // fixes a positioning bug with the localized font sheet
		if( Index < 0 )
		{
			Index += 256;
		}
		// bold characters are on the bottom half ot the texture sheet
		if ( Bold )
		{
			//Index += 128;
		}
		// carriage return zeros our width count
		if( FinalText[i] == 10 )
		{
			if( StartingPosition != 0 )
			{
				StartingPosition -= ( m_pFontMetric->Kerning() ) * Scale;
			}
			LineWidth.push_back( StartingPosition );
			StartingPosition = 0;
		}
		if( FinalText[i] != 8 &&
			FinalText[i] != 10 )
		{
			StartingPosition += ( m_pFontMetric->CharacterWidth( Index ) + m_pFontMetric->Kerning() ) * Scale;
		}
	}
	if( StartingPosition != 0 )
	{
		StartingPosition -= ( m_pFontMetric->Kerning() ) * Scale;
	}
	LineWidth.push_back( StartingPosition );



	uint32 CurrentLine = 0;
	// we assume left justification
	float32	Offset		= 0;
	if( Justification == KJustifyRight )
	{
		Offset -= LineWidth[CurrentLine];
	}
	else if( Justification == KJustifyCenter )
	{
		Offset -= LineWidth[CurrentLine] * .5f;
	}

	// we want it to be centered vertically in the text
	float32	YOffset	= -CharacterWidth * Scale * .5f;

	PLAINVERTEXSCENE*	pVertices;
	// couldn't lock the vert buffer - we failed
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}
	m_MinBounds.x = Offset;
	m_MinBounds.y = -YOffset;
	m_MaxBounds = m_MinBounds;

	bool BoldShift( kFalse );
	D3DXVECTOR3 Vertex;
	for ( i = 0; i < m_TextCharacters; i++ )
	{
		bool	CarriageReturn = kFalse;
		bool	Blank = kFalse;
		int32	Index;
		Index = FinalText[i];

		if( Index == 10 )
		{
			CarriageReturn = kTrue;
			Blank = kTrue;
		}
		if( Index == 8 )
		{
			BoldShift = !BoldShift;
			Index = 10;
			Blank = kTrue;
		}
		// characters start at 32, so we trim off the first 32 indices
		//Index -= 32;
		if( Index < 0 )
		{
			Index += 256;
		}
		float32 ThisCharacterWidth = (float32)m_pFontMetric->CharacterWidth( Index );
		if( Blank )
		{
			ThisCharacterWidth = 0;
		}

		// bold occupies the bottom half of the sheet
		if ( Bold )
		{
			//Index += 128;
		}
		if ( BoldShift )
		{
			//Index += 128;
		}
		// calculate the location of the character in the texture sheet
		int32	Row	= ( int32 ) floor( Index / 16.0f );
		int32	Column	= Index - Row * 16;
		// return character, do a carraige return
		if ( FinalText[i] == '\n' )
		{
			Index = 0;
			Row = 0;
			Column = 0;
		}

		//MSANDIGE, 7/8/08 - copied the following change(s) from the fate gold branch:
		//ECIOCCA, 6/30/08
		//	this is an acknowledged hack to get the bold coloring back in.
		//	ideally if the regular and bold of a font aren't in the same material,
		//	we should have a 2-material method for specifying the bold color. OR a 
		//	way internal to the font to specify a tint palette. 		
		D3DXCOLOR textcolor = (Bold||BoldShift)?D3DXCOLOR( 0.74f, 0.54f, 1.0f, 1.0f ):D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
		// set the vertex and UV locations for this character, 2 polys
		// note that we HAVE to have color on this text. Some card will
		// not render alphaed polys unless they have vertex colors. Wierd, but true.
		Vertex.x = Offset;
		Vertex.z = 0;
		Vertex.y = -YOffset;
		ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );
		pVertices->v = Vertex;		
		pVertices->color = textcolor;
		pVertices->tu = ( Column + .5f ) * CharacterWidth * PixelStep -
						( ThisCharacterWidth * .5f * PixelStep );
		pVertices->tv = Row * CharacterWidth * PixelStep + 1 * PixelStep;
		pVertices++;

		Vertex.x = ( ThisCharacterWidth ) * Scale + Offset;
		Vertex.z = 0;
		Vertex.y = -YOffset;
		ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );
		pVertices->v = Vertex;		
		pVertices->color = textcolor;
		pVertices->tu = ( Column + .5f ) * CharacterWidth * PixelStep +
						( ThisCharacterWidth * .5f * PixelStep );
		pVertices->tv = Row * CharacterWidth * PixelStep + 1 * PixelStep;
		pVertices++;

		Vertex.x = ( ThisCharacterWidth ) * Scale + Offset;
		Vertex.z = 0;
		Vertex.y = -YOffset - CharacterWidth * Scale;
		ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );
		pVertices->v = Vertex;		
		pVertices->color = textcolor;
		pVertices->tu = ( Column + .5f ) * CharacterWidth * PixelStep +
						( ThisCharacterWidth * .5f * PixelStep );
		pVertices->tv = Row * CharacterWidth * PixelStep +
						( CharacterWidth * PixelStep ) - 1 * PixelStep;
		pVertices++;

		Vertex.x = Offset;
		Vertex.z = 0;
		Vertex.y = -YOffset - CharacterWidth * Scale;
		ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );
		pVertices->v = Vertex;		
		pVertices->color = textcolor;
		pVertices->tu = ( Column + .5f ) * CharacterWidth * PixelStep -
						( ThisCharacterWidth * .5f * PixelStep );
		pVertices->tv = Row * CharacterWidth * PixelStep +
						( CharacterWidth * PixelStep ) - 1 * PixelStep;
		pVertices++;

		// not right justified, advance our location to the next character
		if ( !Blank )
		{
			Offset += ( ThisCharacterWidth +
						 m_pFontMetric->Kerning() ) * Scale;
		}

		if( CarriageReturn )
		{
			CurrentLine++;

			Offset		= 0;
			if( Justification == KJustifyRight )
			{
				Offset -= LineWidth[CurrentLine];
			}
			else if( Justification == KJustifyCenter )
			{
				Offset -= LineWidth[CurrentLine] * .5f;
			}
			YOffset += CharacterHeight * Scale;
		}
	}

	m_pVB->Unlock();


	if( CreateNewBuffers )
	{
		// Set up our index buffer
		WORD*	pIndices;
		hr = pD3DDevice->CreateIndexBuffer( Indices * sizeof( WORD ),
											D3DUSAGE_WRITEONLY,
											D3DFMT_INDEX16,
											D3DPOOL_MANAGED,
											&m_pIB );
		// couldn't create the index buffer, return
		if ( FAILED( hr ) )
		{
			return;
		}

		m_pIB->Lock( 0, Indices * sizeof( WORD ), ( BYTE * * ) &pIndices, 0 );
		// couldn't lock the index buffer, return
		if ( FAILED( hr ) )
		{
			return;
		}

		// create the index buffer for the characters
		for ( i = 0; i < m_TextCharacters; i++ )
		{
			*pIndices++ = (WORD)i * 4 + 0;
			*pIndices++ = (WORD)i * 4 + 1;
			*pIndices++ = (WORD)i * 4 + 2;

			*pIndices++ = (WORD)i * 4 + 0;
			*pIndices++ = (WORD)i * 4 + 2;
			*pIndices++ = (WORD)i * 4 + 3;
		}

		m_pIB->Unlock();
	}
} // CText::Update()

void CText::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	// no text, don't render
	if ( m_TextCharacters < 1 )
	{
		return;
	}
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

	if ( m_pMaterial->IsValid() )
	{
		pD3DDevice->SetTexture( 0, m_pMaterial->GetTexture() );
	}	

	D3DXMATRIX	Transformation;
	D3DXMatrixTranslation( &Transformation, m_X, m_Y, 1.0f);
	pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

	pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );
	pD3DDevice->SetIndices( m_pIB, 0L );

	pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
									  0,
									  m_TextCharacters * 4,
									  0,
									  m_TextCharacters * 2 );
} // CText::Render()

// render without textures, if you want to batch these up yourself
void CText::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	// no text, don't render
	if ( m_TextCharacters < 1 )
	{
		return;
	}

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

	pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );
	pD3DDevice->SetIndices( m_pIB, 0L );

	D3DXMATRIX	Transformation;
	D3DXMatrixTranslation( &Transformation, m_X, m_Y, 1.0f);
	pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

	pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
									  0,
									  m_TextCharacters * 4,
									  0,
									  m_TextCharacters * 2 );
} // CText::RenderNoTexture()