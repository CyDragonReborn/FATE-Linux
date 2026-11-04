/*
	bitmap.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _BITMAP_H
#define _BITMAP_H

// inlines for quick color packing

// pack 888 RGB values into a uint16
inline uint16 RGB16( uint32 r, 
					 uint32 g, 
					 uint32 b )
{
	return (uint16)( ( ( ( r >> 3 ) & 0x1F ) << 10 ) |
				     ( ( ( g >> 3 ) & 0x1F ) << 5 ) |
				     ( ( b >> 3 ) & 0x1F ) );
}

// pack 565 RGB values into a uint16
inline uint16 RGB16_565( uint32 r, 
						 uint32 g, 
						 uint32 b )
{
	return (uint16)( ( ( ( ( r >> 3 ) & 0x1F ) << 11 ) | 
				     ( ( ( g >> 2 ) & 0x3F ) << 5 ) ) |
				     ( ( b >> 3 ) & 0x1F ) );
}

// structs for various color formats, used when doing manual copies/merges
// of data from image to image

struct JP2RGB
{
	uint8	Red;
	uint8	Green;
	uint8	Blue;
};

struct JP2RGBA
{
	uint8	Red;
	uint8	Green;
	uint8	Blue;
	uint8	Alpha;
};

struct A8R8G8B8
{
	uint8	Blue;
	uint8	Green;
	uint8	Red;
	uint8	Alpha;
};
struct A4R4G4B4
{
	uint8	Blue	: 4;
	uint8	Green	: 4;
	uint8	Red		: 4;
	uint8	Alpha	: 4;
};
struct A1R5G5B5
{
	uint16	Blue	: 5;
	uint16	Green	: 5;
	uint16	Red		: 5;
	uint16	Alpha	: 1;
};
struct R5G6B5
{
	uint8	Blue	: 5;
	uint8	Green	: 6;
	uint8	Red		: 5;
};
struct R8G8B8
{
	uint8	Blue;
	uint8	Green;
	uint8	Red;
};


// methods for merging/copying data from image to image

// merge into the alpha channel of an A4R4G4B4 image
void MergeAlphaA4R4G4B4( uint16 Width,					// width of the target/source rects 
						 uint16 Height,					// height of the target/source rects
						 D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha
						 D3DLOCKED_RECT* pSourceRect );	// locked rect of the image we're going to copy from



// merge into the alpha channel for cards that can only support 1-bit alpha (yuck!)
void MergeAlphaA1R5G5B5( uint16 Width,					// width of the target/source rects 
						 uint16 Height,					// height of the target/source rects
						 D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha
						 D3DLOCKED_RECT* pSourceRect );	// locked rect of the image we're going to copy from


// merge into the alpha channel of an A8R8G8B8 image
void MergeAlphaA8R8G8B8( uint16 Width,					// width of the target/source rects 
						 uint16 Height,					// height of the target/source rects
						 D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha
						 D3DLOCKED_RECT* pSourceRect );	// locked rect of the image we're going to copy from

// given a list of color data, fill an R8G8B8 texture
void FillTextureR8G8B8( uint16 Width,				// width of the target/source rects
						uint16 Height,				// height of the target/source rects
						D3DLOCKED_RECT* pTargetRect,// locked rect of the image we are going to copy to the alpha 
						uint8* pSourceBitmapData );	// pointer to a list of source data to copy to the locked target

// given a list of color data (with NO alpha component in it) , fill an A8R8G8B8 texture
void FillTextureA8R8G8B8( uint16 Width,					// width of the target/source rects
						  uint16 Height,				// height of the target/source rects
						  D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 						  uint8* pSourceBitmapData );	// pointer to a list of source data to copy to the locked target

// given a list of color data (WITH alpha component in it) , fill an A8R8G8B8 texture
void FillAlphaTextureA8R8G8B8( uint16 Width,				// width of the target/source rects
							   uint16 Height,				// height of the target/source rects
							   D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 							   uint8* pSourceBitmapData );	// pointer to a list of source data to copy to the locked target

// given a list of color data (WITH alpha component in it) , fill an A4R4G4B4 texture
void FillAlphaTextureA4R4G4B4( uint16 Width,				// width of the target/source rects
							   uint16 Height,				// height of the target/source rects
							   D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 							   uint8* pSourceBitmapData );	// pointer to a list of source data to copy to the locked target

// given a list of color data, fill an R5G6B5 texture
void FillTextureR5G6B5( uint16 Width,					// width of the target/source rects
						uint16 Height,					// height of the target/source rects
						D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 						uint8* pSourceBitmapData );		// pointer to a list of source data to copy to the locked target

// given a list of color data (WITH alpha component in it) , fill an A1R5G5B5 texture
void FillAlphaTextureA1R5G5B5( uint16 Width,				// width of the target/source rects
							   uint16 Height,				// height of the target/source rects
							   D3DLOCKED_RECT* pTargetRect,	// locked rect of the image we are going to copy to the alpha 
 							   uint8* pSourceBitmapData );	// pointer to a list of source data to copy to the locked target

#endif