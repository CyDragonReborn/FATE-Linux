/*
	bitmap.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/
#include <D3D8.h>

#include "../UTILITIES/constants.h"
#include "bitmap.h"

// merge into the alpha channel of an A4R4G4B4 image
void MergeAlphaA4R4G4B4( uint16 Width,					// width of the target/source rects 
						 uint16 Height,					// height of the target/source rects
						 D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha
						 D3DLOCKED_RECT* pSourceRect )	// locked rect of the image we're going to copy from
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );
	uint16	TargetPitchDifference( 0 ) ;

	//With this texture format, we have 4 bits for each of the Alpha, Red, Green, and Blue
	//channels.

	//PitchDifference is the amount we have to add to skip to reach the next line
	PitchDifference = ( pTargetRect->Pitch / 2 ) - Width;
	TargetPitchDifference = ( pSourceRect->Pitch / 2 ) - Width;

	A4R4G4B4*	pTarget	= ( A4R4G4B4* ) pTargetRect->pBits;
	A4R4G4B4*	pSource	= ( A4R4G4B4* ) pSourceRect->pBits;

	//Now we loop through the whole texture setting the pSource component equal to the Green component.
	//Since our image was grayscale, we could have used the Blue or Red instead of Green and it
	//would have made no difference.
	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Alpha = pSource->Green;
			pTarget++;
			pSource++;
		}
		//Add PitchDifference to skip to the next line.
		pTarget += PitchDifference;
		pSource += TargetPitchDifference;
	}
} // MergeAlphaA4R4G4B4()

// merge into the alpha channel for cards that can only support 1-bit alpha (yuck!)
void MergeAlphaA1R5G5B5( uint16 Width,					// width of the target/source rects 
						 uint16 Height,					// height of the target/source rects
						 D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha
						 D3DLOCKED_RECT* pSourceRect )	// locked rect of the image we're going to copy from
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );
	uint16	TargetPitchDifference( 0 ) ;

	//With this texture format, we have 1 bit for each of the alpha channel

	//PitchDifference is the amount we have to add to skip to reach the next line
	PitchDifference = ( pTargetRect->Pitch / 2 ) - Width;
	TargetPitchDifference = ( pSourceRect->Pitch / 2 ) - Width;

	uint16 * dest = ( uint16 * ) pTargetRect->pBits;
	uint16 * src = ( uint16 * ) pSourceRect->pBits;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			// taking the blue component, we cut off our alpha at about the halfway point 
			// (less than 128 is transparent, greater is opaque
			uint16 Blue = ( *src ) & 0x1f;
			if ( Blue < 18 )
			{
				( *dest ) &= 0x7fff;
			}

			dest++;
			src++;
		}
		dest += PitchDifference;
		src += TargetPitchDifference;
	}
} // MergeAlphaA1R5G5B5()

void MergeAlphaA8R8G8B8( uint16 Width,					// width of the target/source rects 
						 uint16 Height,					// height of the target/source rects
						 D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha
						 D3DLOCKED_RECT* pSourceRect )	// locked rect of the image we're going to copy from
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );
	uint16	TargetPitchDifference( 0 ) ;

	//With this texture format, we have 8 bits for each of the Alpha, Red, Green, and Blue
	//channels.

	//
	//PitchDifference is the amount we have to add to skip to reach the next line
	PitchDifference = ( pTargetRect->Pitch / 4 ) - Width;
	TargetPitchDifference = ( pSourceRect->Pitch / 4 ) - Width;

	A8R8G8B8*	pTarget	= ( A8R8G8B8* ) pTargetRect->pBits;
	A8R8G8B8*	pSource	= ( A8R8G8B8* ) pSourceRect->pBits;

	//Now we loop through the whole texture setting the pSource component equal to the Green component.
	//Since our image was grayscale, we could have used the Blue or Red instead of Green and it
	//would have made no difference.
	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Alpha = pSource->Green;
			pTarget++;
			pSource++;
		}
		//Add PitchDifference to skip to the next line.
		pTarget += PitchDifference;
		pSource += TargetPitchDifference;
	}
} // MergeAlphaA8R8G8B8()


void FillTextureR8G8B8( uint16 Width,				// width of the target/source rects
						uint16 Height,				// height of the target/source rects
						D3DLOCKED_RECT* pTargetRect,// locked rect of the image we are going to copy to the alpha 
						uint8* pSourceBitmapData )	// pointer to a list of source data to copy to the locked target
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );

	PitchDifference = ( pTargetRect->Pitch / 3 ) - Width;

	R8G8B8* pTarget	= ( R8G8B8* ) pTargetRect->pBits;
	JP2RGB* pSource	= ( JP2RGB* ) pSourceBitmapData;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Red = pSource->Red;
			pTarget->Green = pSource->Green;
			pTarget->Blue = pSource->Blue;
			pTarget++;
			pSource++;
		}
		pTarget += PitchDifference;
	}
} // FillTextureR8G8B8()

void FillTextureA8R8G8B8( uint16 Width,					// width of the target/source rects
						  uint16 Height,				// height of the target/source rects
						  D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 						  uint8* pSourceBitmapData )	// pointer to a list of source data to copy to the locked target

{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );

	PitchDifference = ( pTargetRect->Pitch / 4 ) - Width;

	A8R8G8B8*	pTarget	= ( A8R8G8B8* ) pTargetRect->pBits;
	JP2RGB*		pSource	= ( JP2RGB* ) pSourceBitmapData;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Red = pSource->Red;
			pTarget->Green = pSource->Green;
			pTarget->Blue = pSource->Blue;
			pTarget->Alpha = 255;
			pTarget++;
			pSource++;
		}
		pTarget += PitchDifference;
	}
} // FillTextureA8R8G8B8()

void FillAlphaTextureA8R8G8B8( uint16 Width,				// width of the target/source rects
							   uint16 Height,				// height of the target/source rects
							   D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 							   uint8* pSourceBitmapData )	// pointer to a list of source data to copy to the locked target
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );

	PitchDifference = ( pTargetRect->Pitch / 4 ) - Width;

	A8R8G8B8*	pTarget	= ( A8R8G8B8* ) pTargetRect->pBits;
	JP2RGBA*	pSource	= ( JP2RGBA* ) pSourceBitmapData;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Red = pSource->Red;
			pTarget->Green = pSource->Green;
			pTarget->Blue = pSource->Blue;
			pTarget->Alpha = pSource->Alpha;
			pTarget++;
			pSource++;
		}
		pTarget += PitchDifference;
	}
} // FillAlphaTextureA8R8G8B8()

void FillAlphaTextureA4R4G4B4( uint16 Width,				// width of the target/source rects
							   uint16 Height,				// height of the target/source rects
							   D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 							   uint8* pSourceBitmapData )	// pointer to a list of source data to copy to the locked target
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );

	PitchDifference = ( pTargetRect->Pitch / 2 ) - Width;

	A4R4G4B4*	pTarget	= ( A4R4G4B4* ) pTargetRect->pBits;
	JP2RGBA*	pSource	= ( JP2RGBA* ) pSourceBitmapData;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Red = pSource->Red * 15 / 255;
			pTarget->Green = pSource->Green * 15 / 255;
			pTarget->Blue = pSource->Blue * 15 / 255;
			pTarget->Alpha = pSource->Alpha * 15 / 255;
			pTarget++;
			pSource++;
		}
		pTarget += PitchDifference;
	}
} //FillAlphaTextureA4R4G4B4()


void FillTextureR5G6B5( uint16 Width,					// width of the target/source rects
						uint16 Height,					// height of the target/source rects
						D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 						uint8* pSourceBitmapData )		// pointer to a list of source data to copy to the locked target
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );

	PitchDifference = ( pTargetRect->Pitch / 2 ) - Width;


	uint16* dest	= ( uint16 * ) pTargetRect->pBits;
	JP2RGB* pSource	= ( JP2RGB* ) pSourceBitmapData;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			( *dest ) = RGB16_565( pSource->Red, pSource->Green, pSource->Blue );
			dest++;
			pSource++;
		}
		dest += PitchDifference;
	}
} // FillTextureR5G6B5()

void FillAlphaTextureA1R5G5B5( uint16 Width,				// width of the target/source rects
							   uint16 Height,				// height of the target/source rects
							   D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 							   uint8* pSourceBitmapData )	// pointer to a list of source data to copy to the locked target
{
	uint16	X( 0 );
	uint16  Y( 0 );
	uint16	PitchDifference( 0 );

	PitchDifference = ( pTargetRect->Pitch / 2 ) - Width;

	A1R5G5B5*	pTarget	= ( A1R5G5B5* ) pTargetRect->pBits;
	JP2RGBA*	pSource	= ( JP2RGBA* ) pSourceBitmapData;

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( X = 0; X < Width; X++ )
		{
			pTarget->Red = pSource->Red;
			pTarget->Green = pSource->Green;
			pTarget->Blue = pSource->Blue;
			pTarget->Alpha = pSource->Alpha;
			pTarget++;
			pSource++;
		}
		pTarget += PitchDifference;
	}
} // FillAlphaTextureA1R5G5B5()
