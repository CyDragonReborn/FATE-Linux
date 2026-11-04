/*
	fontmetric.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
	CHANGE HISTORY
	7/15/2004	-	Padding was cumulative. Has now been made absolute.
*/

#ifndef _FONTMETRIC_H
#define _FONTMETRIC_H

#include <string>
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/textfile.h"

// maintains text widths for a given font
class CFontMetric
{
public:
	CFontMetric( std::string FileName ) :		// name of font metric file
				 m_Kerning( 0 ),
				 m_Padding( 0 ),
				 m_CharacterHeight( -1 )
	{
		ZeroWidth();

		if( !FileExists( FileName ) ) return;

		CTextFile	File;
		File.Open( FileName.c_str() );
		std::queue <std::string>	Tokens;
		
		bool LoadComplete	= kFalse;
		while ( !LoadComplete )
		{
			std::string	TokenValue;
			File.TokenizeNextLine( &Tokens,
				" \t\n\r:=",
				'#' );	
			if ( File.IsEOF() )
			{
				break;
			}
			if ( Tokens.size() <= 0 )
			{
				continue;
			}
				
			if ( Tokens.front() != "[Char" )
			{
				int32	Index	= atoi( Tokens.front().c_str() );
				Tokens.pop();

				if ( Tokens.size() > 0 && Index <= 255 )
				{
					int32	Width	= atoi( Tokens.front().c_str() );
					m_Width[Index] = Width;
				}
			}
			ClearQueue( Tokens );
		}		
			
		File.Close();
	} // CFontMetric( )

	~CFontMetric()
	{
	} // ~CFontMetric()

	void ZeroWidth()
	{
		for( uint32 i = 0; i < 256; i++ )
		{
			m_Width[i] = 0;
		}
	}

	void Pad( int32 Pixels )		// pixels to pad the width of every character
	{
		m_Padding = Pixels;

	} // Pad()

	// set 'kerning' (the spacing between characters) in pixels
	void	SetKerning( float32 Pixels )			{	m_Kerning = Pixels;			};

	void	SetCharacterHeight( float32 Pixels )	{	m_CharacterHeight = Pixels;	};

	uint32	CharacterWidth( uint32 Character )
	{
		if( Character < 256 )
		{
			return m_Width[Character] + m_Padding;
		}
		return 0;
	} // CharacterWidth()

	float32 CharacterHeight( void )			{	return m_CharacterHeight;	};

	float32 Kerning( void )					{	return m_Kerning;	};

private:

	int32		m_Width[256];
	float32		m_Kerning;
	int32		m_Padding;
	float32		m_CharacterHeight;
};

#endif