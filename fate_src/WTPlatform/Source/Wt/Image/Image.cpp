/***************************************************************************************
 *
 * Image.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 6/28/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

//TODO provide the ability to assemble a 3D image from multiple compatible 2D images.
//TODO provide the ability to load a cube map from 6 seperate 2D files.

#include "WT/stdafx.h"

#include <cassert>
#include <vector>
#include "WT/System/Platform.h"
#include "WT/System/Logging/Log.h"
#include "WT/System/IO/FileStream.h"
#include "WT/System/IO/File.h"
#include "WT/Image/Image.h"
#include "WT/Math/Math.h"
#include "WT/Math/LinearAlgebra/Vector4f.h"

namespace WT
{
namespace Graphics
{

using namespace Math;

WT_DEFINE_CLASS( CImageLock )
WT_DEFINE_CLASS( CImage )
WT_DEFINE_CLASS( CImageSerializer )

uint8* CImageLock::GetDataPtr() const
{
	return m_pData;
}

CImageLock::~CImageLock()
{
	assert( m_hImage != NULL );
	if (m_bPalette)
	{
		m_hImage->UnlockPalette();
	} else {
		m_hImage->Unlock();
	}
}

CImageLock::CImageLock( CImage& Image, uint8* pData, bool bPalette )
{
	m_hImage = &Image;
	m_pData = pData;
	m_bPalette = bPalette;
}

float32 ComputeLuminance( float32 r, float32 g, float32 b )
{
	return r * 0.222491f + g * 0.716888f + b * 0.060621f;
}

CImage::~CImage()
{
    assert( !m_bImageDataLocked ); //~Image() reached before Unlock() called.
    assert( !m_bPaletteDataLocked ); //~Image() reached before UnlockPalette() called.
	assert( m_hMemoryDC == NULL ); //~Image() reached before UnlockDC() called.

	ReleaseDC();

	DeleteCriticalSection( &m_Mutex );
}

CImage::CImage()
{
    m_Desc.Width = 0;
    m_Desc.Height = 0;
	m_Desc.Depth = 0;
    m_Desc.PaletteSize = 0;
    m_Desc.RowPitch = 0;
    m_Desc.SlicePitch = 0;
	m_Desc.ImagePitch = 0;
    m_Desc.Format = EPixelFormat::None; 
    m_bImageDataLocked = false;
    m_bPaletteDataLocked = false;
	m_hMemoryDC = NULL;
	m_hBitmap = NULL;
	m_hOrigObj = NULL;
	InitializeCriticalSection( &m_Mutex );
}

CImage::CImage( CImage const& image )
{
    m_Desc.Width = 0;
    m_Desc.Height = 0;
	m_Desc.Depth = 0;
    m_Desc.PaletteSize = 0;
    m_Desc.RowPitch = 0;
    m_Desc.SlicePitch = 0;
	m_Desc.ImagePitch = 0;
    m_Desc.Format = EPixelFormat::None; 
    m_bImageDataLocked = false;
    m_bPaletteDataLocked = false;
	m_hMemoryDC = NULL;
	m_hBitmap = NULL;
	m_hOrigObj = NULL;
	InitializeCriticalSection( &m_Mutex );

	Init( image.GetDesc() );

	//Copy palette and texture data from pImage to Result
	
	HImageLock hSourceLock = const_cast<CImage&>( image ).Lock( EGraphicsResourceLock::ReadOnly );
	uint8 const* pSource = hSourceLock->GetDataPtr();

	HImageLock hSourceLockPal = const_cast<CImage&>( image ).LockPalette( EGraphicsResourceLock::ReadOnly );
	uint8 const* pSourcePal = hSourceLockPal->GetDataPtr();

	HImageLock hDestLock = Lock( EGraphicsResourceLock::WriteOnly );
	uint8* pDest = hDestLock->GetDataPtr();

	HImageLock hDestLockPal = LockPalette( EGraphicsResourceLock::WriteOnly );
	uint8* pDestPal = hDestLockPal->GetDataPtr();

	if (m_Desc.PaletteSize)
	{
		assert( pSourcePal != NULL );
		assert( pDestPal != NULL );
		memcpy( pDestPal, pSourcePal, m_Desc.PaletteSize * 3 );
	}

	memcpy( pDest, pSource, m_Desc.ImagePitch );
}

void CImage::Init(Desc const& Desc, uint32 ARGB)
{
    if (Desc.Format < 0 || Desc.Format >= EPixelFormat::Count ||
        Desc.Width < 0 || Desc.Height < 0 || Desc.Depth < 0 ||
        Desc.PaletteSize < 0)
    {
		throw std::out_of_range( "image init arguments" );
    }

    if (m_bImageDataLocked || m_bPaletteDataLocked)
    {
		throw std::runtime_error( "image locked" );
    }

    //For non-empty formats, nonzero dimensions are required.
    if (Desc.Format != EPixelFormat::None &&
        (Desc.Width <= 0 || Desc.Height <= 0 || Desc.Depth <= 0))
    {
		throw std::out_of_range( "zero image size" );
    }

	bool bBlocks = false; //4x4 blocks (used with DXT compression)?
	int32 BytesPerPixel = 0;
	int32 BytesPerPalEnt = 0;

    switch (Desc.Format)
    {
    default:
        assert( false );

    case EPixelFormat::None:
        if (Desc.Width != 0 ||
            Desc.Height != 0 ||
			Desc.Depth != 0 ||
            Desc.PaletteSize != 0)
        {
			throw std::out_of_range( "empty image details" );
        }
        break;
    case EPixelFormat::X8R8G8B8:
        if (Desc.PaletteSize != 0)
        {
			throw std::out_of_range( "image palette size" );
        }
        BytesPerPixel = 4;
        break;
    case EPixelFormat::A8R8G8B8:
        if (Desc.PaletteSize != 0)
        {
			throw std::out_of_range( "image palette size" );
        }
        BytesPerPixel = 4;
        break;
    case EPixelFormat::L8:
        if (Desc.PaletteSize != 0)
        {
			throw std::out_of_range( "image palette size" );
        }
        BytesPerPixel = 1;
        break;
    case EPixelFormat::P8_R8G8B8:
        if (Desc.PaletteSize <= 0 ||
            Desc.PaletteSize > 256)
        {
			throw std::out_of_range( "image palette size" );
        }
        BytesPerPixel = 1;
        BytesPerPalEnt = 3;
        break;
	case EPixelFormat::R32F:
		if (Desc.PaletteSize != 0)
		{
			throw std::out_of_range( "image palette size" );
		}
		BytesPerPixel = 4;
		break;
	case EPixelFormat::A32B32G32R32F:
		if (Desc.PaletteSize != 0)
		{
			throw std::out_of_range( "image palette size" );
		}
		BytesPerPixel = 16;
		break;
	case EPixelFormat::A16B16G16R16F:
		if (Desc.PaletteSize != 0)
		{
			throw std::out_of_range( "image palette size" );
		}
		BytesPerPixel = 8;
		break;
	case EPixelFormat::DXT1:
		if (Desc.PaletteSize != 0)
		{
			throw std::out_of_range( "image palette size" );
		}
		bBlocks = true;
		BytesPerPixel = 0; //Note: 0.5 actually...
		break;
	case EPixelFormat::DXT3:
	case EPixelFormat::DXT5:
		if (Desc.PaletteSize != 0)
		{
			throw std::out_of_range( "image palette size" );
		}
		bBlocks = true;
		BytesPerPixel = 1;
		break;
    }

    m_Desc = Desc;
	if (bBlocks)
	{
		//Adjust for 4x4 blocks
		m_Desc.RowPitch = (((m_Desc.Width + 3) & ~3) * (BytesPerPixel + 1)) >> 1; //Effective bytes per row
		m_Desc.SlicePitch = (m_Desc.Height + 3) & ~3;
	} else {
	    m_Desc.RowPitch = m_Desc.Width * BytesPerPixel;
		m_Desc.SlicePitch = m_Desc.Height;
	}
	m_Desc.SlicePitch *= m_Desc.RowPitch;
	m_Desc.ImagePitch = m_Desc.SlicePitch * m_Desc.Depth;
    
	m_Data.clear();
	m_Palette.clear();

    m_Data.resize( m_Desc.ImagePitch );
    //If indexed, initialize the image data (indices) to zeroes
    m_Palette.resize( m_Desc.PaletteSize * BytesPerPalEnt, 0 );

	if (bBlocks)
	{
		//Just initialize the memory. Do not guarantee any particular color for now.
		std::fill( m_Data.begin(), m_Data.end(), 0 );
	} else {
	    FillColor( ARGB );
	}
}

CImage::Desc const& CImage::GetDesc() const
{
    return m_Desc;
}

int32 CImage::GetWidth() const
{
    return m_Desc.Width;
}

int32 CImage::GetHeight() const
{
    return m_Desc.Height;
}

int32 CImage::GetDepth() const
{
	return m_Desc.Depth;
}

int32 CImage::GetPaletteSize() const
{
    return m_Desc.PaletteSize;
}

int32 CImage::GetRowPitch() const
{
    return m_Desc.RowPitch;
}

int32 CImage::GetSlicePitch() const
{
    return m_Desc.SlicePitch;
}

int32 CImage::GetImagePitch() const
{
	return m_Desc.ImagePitch;
}

EPixelFormat::Type CImage::GetFormat( ) const
{
    return m_Desc.Format;
}

HImageLock CImage::Lock( EGraphicsResourceLock::Type LockFlags )
{
    if (LockFlags < 0 || LockFlags > EGraphicsResourceLock::ReadWrite)
    {
		throw std::out_of_range( "lock flags" );
    }

    {
		EnterCriticalSection( &m_Mutex );

        if (m_bImageDataLocked)
        {
			LeaveCriticalSection( &m_Mutex );
			throw std::runtime_error( "already locked" );
        }
    
        m_bImageDataLocked = true;
		LeaveCriticalSection( &m_Mutex );
    }

	HImageLock hImageLock = new CImageLock( *this, &m_Data[0], false );
	return hImageLock;
}

void CImage::Unlock( )
{
    if (!m_bImageDataLocked)
    {
		throw std::runtime_error( "not locked" );
    }
    m_bImageDataLocked = false;
}

bool CImage::IsLocked() const
{
    return m_bImageDataLocked;
}

HImageLock CImage::LockPalette( EGraphicsResourceLock::Type LockFlags )
{
    if (LockFlags < 0 || LockFlags > EGraphicsResourceLock::ReadWrite)
    {
		throw std::out_of_range( "lock flags" );
    }

    {
		EnterCriticalSection( &m_Mutex );

        if (m_bPaletteDataLocked)
        {
			LeaveCriticalSection( &m_Mutex );
			throw std::runtime_error( "already locked" );
        }

        m_bPaletteDataLocked = true;
		LeaveCriticalSection( &m_Mutex );
    }

	HImageLock hImageLock = new CImageLock( *this, &m_Palette[0], true );
	return hImageLock;
}

void CImage::UnlockPalette( )
{
    if (!m_bPaletteDataLocked)
    {
		throw std::runtime_error( "not locked" );
    }
    m_bPaletteDataLocked = false;
}

bool CImage::IsPaletteLocked() const
{
    return m_bPaletteDataLocked;
}

uint32 CImage::GetPixelColor( int32 X, int32 Y, int32 Z ) const
{
	//TODO fail if writable locks exist.
	//TODO establish read-only locks and ensure thread safety.
	
	if ((X | Y | Z) < 0 || X >= m_Desc.Width || Y >= m_Desc.Height || Z >= m_Desc.Depth)
	{
		return 0xffffffffu;
	}

	uint8 const* pRow = &m_Data[Z * m_Desc.SlicePitch + Y * m_Desc.RowPitch];

    switch (m_Desc.Format)
    {
    case EPixelFormat::X8R8G8B8:
    case EPixelFormat::A8R8G8B8:
		assert( !m_Data.empty() );
		return reinterpret_cast<uint32 const*>( pRow )[X];

    case EPixelFormat::L8:
		{
			assert( !m_Data.empty() );
			uint32 Color = pRow[X];
			return 0xff000000u + (Color << 16) + (Color << 8) + Color;
		}

    case EPixelFormat::P8_R8G8B8:
		{
			assert( !m_Data.empty() );
			uint32 PaletteIndex = pRow[X];
			assert( PaletteIndex < static_cast<uint32>( m_Desc.PaletteSize ) );
			assert( !m_Palette.empty() );
			uint8 const* pPalEntry = &m_Palette[PaletteIndex * 3];
			return 0xff000000u 
				+ (static_cast<uint32>( pPalEntry[0] ) << 16)
				+ (static_cast<uint32>( pPalEntry[1] ) <<  8)
				+ (static_cast<uint32>( pPalEntry[2] )		 );
		}

	case EPixelFormat::R32F:
		{
			assert( !m_Data.empty() );
			float32 const fR = reinterpret_cast<float32 const*>( pRow )[X];
			int32 r = Float32ToInt32( fR );
			if (r < 0) r = 0; else if (r > 255) r = 255;
			return 0xff00ffffu + (r << 16);
		}
	case EPixelFormat::A32B32G32R32F:
		{
			assert( !m_Data.empty() );
			float32 const fR = reinterpret_cast<float32 const*>( pRow )[4 * X + 0];
			float32 const fG = reinterpret_cast<float32 const*>( pRow )[4 * X + 1];
			float32 const fB = reinterpret_cast<float32 const*>( pRow )[4 * X + 2];
			float32 const fA = reinterpret_cast<float32 const*>( pRow )[4 * X + 3];
			int32 r = Float32ToInt32( fR );
			if (r < 0) r = 0; else if (r > 255) r = 255;
			int32 g = Float32ToInt32( fG );
			if (g < 0) g = 0; else if (g > 255) g = 255;
			int32 b = Float32ToInt32( fB );
			if (b < 0) b = 0; else if (b > 255) b = 255;
			int32 a = Float32ToInt32( fA );
			if (a < 0) a = 0; else if (a > 255) a = 255;
			return (a << 24) + (r << 16) + (g << 8) + b;
		}
	case EPixelFormat::A16B16G16R16F:
		{
			assert( !m_Data.empty() );
			float32 const fR = Float16ToFloat32( reinterpret_cast<float16 const*>( pRow )[4 * X + 0] );
			float32 const fG = Float16ToFloat32( reinterpret_cast<float16 const*>( pRow )[4 * X + 1] );
			float32 const fB = Float16ToFloat32( reinterpret_cast<float16 const*>( pRow )[4 * X + 2] );
			float32 const fA = Float16ToFloat32( reinterpret_cast<float16 const*>( pRow )[4 * X + 3] );
			int32 r = Float32ToInt32( fR );
			if (r < 0) r = 0; else if (r > 255) r = 255;
			int32 g = Float32ToInt32( fG );
			if (g < 0) g = 0; else if (g > 255) g = 255;
			int32 b = Float32ToInt32( fB );
			if (b < 0) b = 0; else if (b > 255) b = 255;
			int32 a = Float32ToInt32( fA );
			if (a < 0) a = 0; else if (a > 255) a = 255;
			return (a << 24) + (r << 16) + (g << 8) + b;
		}
	}

	return 0xffffffffu;
}

void CImage::SetPixelColor( int32 X, int32 Y, int32 Z, uint32 ARGB )
{
	//TODO fail if locks exist.
	//TODO establish writable locks and ensure thread safety.

	if ((X | Y | Z) < 0 || X >= m_Desc.Width || Y >= m_Desc.Height || Z >= m_Desc.Depth)
	{
		throw std::out_of_range( "pixel coordinates" );
	}

	uint8* pRow = &m_Data[Z * m_Desc.SlicePitch + Y * m_Desc.RowPitch];

	switch (m_Desc.Format)
    {
    case EPixelFormat::X8R8G8B8:
    case EPixelFormat::A8R8G8B8:
		assert( !m_Data.empty() );
		reinterpret_cast<uint32*>( pRow )[X] = ARGB;
		return;

    case EPixelFormat::L8:
		{
			uint32 Color = ARGB & 0xffu;

			ARGB ^= Color;
			ARGB ^= (Color << 8);
			ARGB ^= (Color << 16);
			if (ARGB != 0xff000000u)
			{
				throw std::invalid_argument( "translucent or non-grayscale color" );
			}

			assert( !m_Data.empty() );
			pRow[X] = static_cast<uint8>( Color );
			return;
		}

    case EPixelFormat::P8_R8G8B8:
		{
			//Only opaque colors allowed
			if (ARGB < 0xff000000u)
			{
				throw std::invalid_argument( "translucent color" );
			}

			ARGB &= 0x00ffffffu;

			assert( !m_Palette.empty() );
			uint32 PaletteIndex = 0;
			for (; PaletteIndex < static_cast<uint32>( m_Desc.PaletteSize ); ++PaletteIndex)
			{
				uint8* pPalEntry = &m_Palette[PaletteIndex * 3];
				uint32 PalColor = 
					(static_cast<uint32>( pPalEntry[0] ) << 16) |
					(static_cast<uint32>( pPalEntry[1] ) <<  8) |
					(static_cast<uint32>( pPalEntry[2] )	  );

				if (PalColor == ARGB)
				{
					break;
				}
			}

			if (PaletteIndex >= static_cast<uint32>( m_Desc.PaletteSize ))
			{
				throw std::invalid_argument( "non-representable color" );
			}

			assert( !m_Data.empty() );
			assert( m_Desc.PaletteSize <= 255 );
			pRow[X] = static_cast<uint8>( PaletteIndex );
			return;
		}

    case EPixelFormat::R32F:
		{
			if ((ARGB & 0xff00ffffu) != 0xff00ffffu)
			{
				throw std::invalid_argument( "non-red color" );
			}

			assert( !m_Data.empty() );
			reinterpret_cast<float32*>( pRow )[X] = (ARGB & 0x00ff0000u) * (1.0f / (1 << 16));
			return;
		}
    case EPixelFormat::A32B32G32R32F:
		{
			assert( !m_Data.empty() );
			reinterpret_cast<float32*>( pRow )[4 * X + 0] = (ARGB & 0x00ff0000u) * (1.0f / (1 << 16));
			reinterpret_cast<float32*>( pRow )[4 * X + 1] = (ARGB & 0x0000ff00u) * (1.0f / (1 <<  8));
			reinterpret_cast<float32*>( pRow )[4 * X + 2] = (ARGB & 0x000000ffu) * (1.0f / (1 <<  0));
			reinterpret_cast<float32*>( pRow )[4 * X + 3] = (ARGB & 0xff000000u) * (1.0f / (1 << 24));
			return;
		}
    case EPixelFormat::A16B16G16R16F:
		{
			assert( !m_Data.empty() );
			reinterpret_cast<float16*>( pRow )[4 * X + 0] = Float32ToFloat16( (ARGB & 0x00ff0000u) * (1.0f / (1 << 16)) );
			reinterpret_cast<float16*>( pRow )[4 * X + 1] = Float32ToFloat16( (ARGB & 0x0000ff00u) * (1.0f / (1 <<  8)) );
			reinterpret_cast<float16*>( pRow )[4 * X + 2] = Float32ToFloat16( (ARGB & 0x000000ffu) * (1.0f / (1 <<  0)) );
			reinterpret_cast<float16*>( pRow )[4 * X + 3] = Float32ToFloat16( (ARGB & 0xff000000u) * (1.0f / (1 << 24)) );
			return;
		}
	}

	throw std::runtime_error( "unsupported format" );
}

int32 CImage::GetPixelPaletteIndex( int32 X, int32 Y, int32 Z ) const
{
	//TODO fail if writable locks exist.
	//TODO establish read-only locks and ensure thread safety.
	
	if ((X | Y | Z) < 0 || X >= m_Desc.Width || Y >= m_Desc.Height || Z >= m_Desc.Depth)
	{
		return -1;
	}

	switch (m_Desc.Format)
    {
    case EPixelFormat::P8_R8G8B8:
		{
			assert( !m_Data.empty() );
			uint8 const* pRow = &m_Data[Z * m_Desc.SlicePitch + Y * m_Desc.RowPitch];
			uint32 PaletteIndex = pRow[X];
			assert( PaletteIndex < static_cast<uint32>( m_Desc.PaletteSize ) );
			return PaletteIndex;
		}
	}

	return -1;
}

void CImage::SetPixelPaletteIndex( int32 X, int32 Y, int32 Z, uint32 PaletteIndex )
{
	//TODO fail if locks exist.
	//TODO establish writable locks and ensure thread safety.

	if ((X | Y | Z) < 0 || X >= m_Desc.Width || Y >= m_Desc.Height || Z >= m_Desc.Depth)
	{
		throw std::out_of_range( "pixel coordinates" );
	}

    switch (m_Desc.Format)
    {
    case EPixelFormat::P8_R8G8B8:
		{
			if (PaletteIndex >= static_cast<uint32>( m_Desc.PaletteSize ))
			{
				throw std::out_of_range( "palette index" );
			}

			assert( !m_Data.empty() );
			assert( m_Desc.PaletteSize <= 255 );
			uint8* pRow = &m_Data[Z * m_Desc.SlicePitch + Y * m_Desc.RowPitch];
			pRow[X] = static_cast<uint8>( PaletteIndex );
			return;
		}
	}

	throw std::runtime_error( "not a paletted image" );
}

/// Locks the image and returns a device context.
HDC CImage::LockDC()
{
	if (m_hMemoryDC != NULL)
	{
		return NULL;
	}

	assert( m_hBitmap == NULL );
	assert( m_hOrigObj == NULL );

	switch (m_Desc.Format)
	{
	    case EPixelFormat::L8:
	    case EPixelFormat::P8_R8G8B8:
	    case EPixelFormat::X8R8G8B8:
		case EPixelFormat::A8R8G8B8:
	    case EPixelFormat::R32F:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
			break;

		default:
			return NULL; //Unsupported format
	}

	m_hMemoryDC = CreateCompatibleDC( NULL );

	if (m_hMemoryDC == NULL)
	{
		return NULL;
	}

	//Copy the data into the bitmap

	std::vector<uint8> Mem;
	Mem.resize( m_Desc.Width * m_Desc.Height * 3 + sizeof( DWORD ) - 1 );

	//DWORD-aligned scan lines
	uint8* pData = reinterpret_cast<uint8*>( (reinterpret_cast<ptrdiff_t>( &Mem[0] ) + sizeof( DWORD ) - 1) & ~(sizeof( DWORD ) - 1) );
	uint8* pOut = pData;

	BITMAPINFO Info;
    memset( &Info, 0, sizeof Info );
    Info.bmiHeader.biSize        = sizeof Info.bmiHeader;
    Info.bmiHeader.biWidth		 = m_Desc.Width;
	Info.bmiHeader.biHeight		 = m_Desc.Height;
    Info.bmiHeader.biPlanes      = 1;
    Info.bmiHeader.biCompression = BI_RGB;
    Info.bmiHeader.biBitCount    = 24;

	// Create the bitmap
	m_hBitmap = ::CreateDIBSection( m_hMemoryDC, &Info, DIB_RGB_COLORS, (void**)&pOut, NULL, 0 );

	// Select the bitmap as the current device object
	m_hOrigObj = ::SelectObject( m_hMemoryDC, m_hBitmap );

	for (int32 i = 0; i < m_Desc.Height; ++i)
	{
		switch (m_Desc.Format)
		{
		default:
			assert( 0 );

	    case EPixelFormat::L8:
			{
				uint8 const* pSource = &m_Data[(m_Desc.Height - 1 - i) * m_Desc.Width];
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					uint8 const L = *pSource++;
					pOut[0] = L;
					pOut[1] = L;
					pOut[2] = L;
					pOut += 3;
				}
			}
		    break;

	    case EPixelFormat::P8_R8G8B8:
			{
				uint8 const* pSource = &reinterpret_cast<uint8*>(&m_Data[0])[(m_Desc.Height - 1 - i) * m_Desc.Width];
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					uint32 Index = *pSource++;
					pOut[0] = m_Palette[Index];
					pOut[1] = m_Palette[Index + 1];
					pOut[2] = m_Palette[Index + 2];
					pOut += 3;
				}
			}
			break;

	    case EPixelFormat::X8R8G8B8:
		case EPixelFormat::A8R8G8B8:
			{
				uint8 const* pSource = reinterpret_cast<uint8 const*>( &reinterpret_cast<uint32 const*>( &m_Data[0] )
					[(m_Desc.Height - 1 - i) * m_Desc.Width] );
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					pOut[0] = pSource[0];
					pOut[1] = pSource[1];
					pOut[2] = pSource[2];
					pOut += 3;
					pSource += sizeof( uint32 );
			    }
			}
		    break;

	    case EPixelFormat::R32F:
			{
				float32 const* pSource = &reinterpret_cast<float32 const*>( &m_Data[0] )[(m_Desc.Height - 1 - i) * m_Desc.Width];
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					pOut[0] = 0xffu;
					pOut[1] = 0xffu;
					int32 r = Float32ToInt32( *pSource++ );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					pOut[2] = static_cast<uint8>( r );
					pOut += 3;
				}
			}
			break;

		case EPixelFormat::A32B32G32R32F:
			{
				float32 const* pSource = &reinterpret_cast<float32 const*>( &m_Data[0] )[(m_Desc.Height - 1 - i) * m_Desc.Width];
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( pSource[0] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					int32 g = Float32ToInt32( pSource[1] );
					if (g < 0) g = 0; else if (g > 255) g = 255;
					int32 b = Float32ToInt32( pSource[2] );
					if (b < 0) b = 0; else if (b > 255) b = 255;
					pOut[0] = static_cast<uint8>( b );
					pOut[1] = static_cast<uint8>( g );
					pOut[2] = static_cast<uint8>( r );
					pOut += 3;
					pSource += 4;
				}
			}
			break;

	    case EPixelFormat::A16B16G16R16F:
			{
				float16 const* pSource = &reinterpret_cast<float16 const*>( &m_Data[0] )[(m_Desc.Height - 1 - i) * m_Desc.Width];
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( Float16ToFloat32( pSource[0] ) );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					int32 g = Float32ToInt32( Float16ToFloat32( pSource[1] ) );
					if (g < 0) g = 0; else if (g > 255) g = 255;
					int32 b = Float32ToInt32( Float16ToFloat32( pSource[2] ) );
					if (b < 0) b = 0; else if (b > 255) b = 255;
					pOut[0] = static_cast<uint8>( b );
					pOut[1] = static_cast<uint8>( g );
					pOut[2] = static_cast<uint8>( r );
					pOut += 3;
					pSource += 4;
				}
			}
			break;
		}
	}

	if (SetDIBits( m_hMemoryDC, m_hBitmap, 0, m_Desc.Height, static_cast<LPVOID>( pData ), &Info, DIB_RGB_COLORS ) != m_Desc.Height)
	{
		ReleaseDC();
		return NULL;
	}

	return m_hMemoryDC;
}

/// Unlocks the image and releases the device context. Not applicable to all platforms.
void CImage::UnlockDC()
{
	if (m_hBitmap == NULL)
	{
		throw std::runtime_error( "device context not locked" );
	}

	//Copy the data from the bitmap
	
	std::vector<uint8> Mem;
	Mem.resize( m_Desc.Width * m_Desc.Height * 3 + sizeof( DWORD ) - 1 );

	//DWORD-aligned scan lines
	uint8* pData = reinterpret_cast<uint8*>( (reinterpret_cast<ptrdiff_t>( &Mem[0] ) + sizeof( DWORD ) - 1) & ~(sizeof( DWORD ) - 1) );

	BITMAPINFO Info;
	memset( &Info, 0, sizeof Info );
	Info.bmiHeader.biSize = sizeof Info.bmiHeader;
	Info.bmiHeader.biWidth = m_Desc.Width;
	Info.bmiHeader.biHeight = m_Desc.Height;
	Info.bmiHeader.biPlanes = 1;
	Info.bmiHeader.biBitCount = 24;
	Info.bmiHeader.biCompression = BI_RGB;
	Info.bmiHeader.biXPelsPerMeter = 3000;
	Info.bmiHeader.biYPelsPerMeter = 3000;

	if (GetDIBits( m_hMemoryDC, m_hBitmap, 0, m_Desc.Height, static_cast<LPVOID>( pData ), &Info, DIB_RGB_COLORS ) != m_Desc.Height)
	{
		ReleaseDC();
		throw std::runtime_error( "GetDIBits() failure" );
	}

	for (int32 i = 0; i < m_Desc.Height; ++i)
	{
		uint8 const* pSource = &pData[(m_Desc.Height - 1 - i) * m_Desc.Width * 3];

		switch (m_Desc.Format)
		{
		default:
			//Should not have locked unsupported format, or should not have changed format
			//while locked.
			assert( false );
			break;

		case EPixelFormat::L8:
			for (int32 j = 0; j < m_Desc.Width; ++j)
			{
				reinterpret_cast<uint8*>( &m_Data[0] )[i * m_Desc.Width + j] = *pSource;
				pSource += 3;
			}
			break;

		case EPixelFormat::P8_R8G8B8:
			//Not implemented
			//TODO implement this
			break;

		case EPixelFormat::X8R8G8B8:
		case EPixelFormat::A8R8G8B8:
			{
				uint32* pOut = reinterpret_cast<uint32*>( &m_Data[0] + (i * m_Desc.RowPitch) );
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					*pOut++ = 0xff000000 + (pSource[2] << 16) +  (pSource[1] << 8) + pSource[0];
					pSource += 3;
				}
			}
			break;

		case EPixelFormat::R32F:
			{
				float32* pOut = reinterpret_cast<float32*>( &m_Data[0] + (i * m_Desc.RowPitch) );
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					*pOut++ = pSource[2];
					pSource += 3;
				}
			}
			break;

		case EPixelFormat::A32B32G32R32F:
			{
				float32* pOut = reinterpret_cast<float32*>( &m_Data[0] + (i * m_Desc.RowPitch) );
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					pOut[0] = pSource[2];
					pOut[1] = pSource[1];
					pOut[2] = pSource[0];
					pOut[3] = 255.0f;
					pSource += 3;
					pOut += 4;
				}
			}
			break;

		case EPixelFormat::A16B16G16R16F:
			{
				float16* pOut = reinterpret_cast<float16*>( &m_Data[0] + (i * m_Desc.RowPitch) );
				for (int32 j = 0; j < m_Desc.Width; ++j)
				{
					pOut[0] = Float32ToFloat16( pSource[2] );
					pOut[1] = Float32ToFloat16( pSource[1] );
					pOut[2] = Float32ToFloat16( pSource[0] );
					pOut[3] = Float32ToFloat16( 255.0f );
					pSource += 3;
					pOut += 4;
				}
			}
			break;
		}
	}

	ReleaseDC();
}

void CImage::ReleaseDC()
{
	if (m_hMemoryDC == NULL)
	{
		assert( m_hBitmap == NULL );
		assert( m_hOrigObj == NULL );
		return;
	}

	assert( m_hBitmap != NULL );

	SelectObject( m_hMemoryDC, m_hOrigObj );
	m_hOrigObj = NULL;
	DeleteObject( m_hBitmap );
	m_hBitmap = NULL;
	DeleteDC( m_hMemoryDC );
	m_hMemoryDC = NULL;
}

void CImage::FillColor(uint32 ARGB)
{
    if (m_bImageDataLocked || m_bPaletteDataLocked)
    {
		throw std::runtime_error( "image locked" );
    }

	uint8* pData = NULL;
	if (!m_Data.empty()) pData = &m_Data[0];
	uint8* pPalette = NULL;
	if (!m_Palette.empty()) pPalette = &m_Palette[0];
	uint32 const ImagePitch = m_Desc.ImagePitch;

	switch (m_Desc.Format)
    {
    case EPixelFormat::None:
        break;
    case EPixelFormat::L8:
        memset( &pData[0], ARGB & 0xff, ImagePitch );
        break;
    case EPixelFormat::P8_R8G8B8:
        {
            uint8 const R = static_cast<uint8>(ARGB >> 16);
            uint8 const G = static_cast<uint8>(ARGB >> 8);
            uint8 const B = static_cast<uint8>(ARGB);
            for (int32 i = 0; i < m_Desc.PaletteSize; ++i)
            {
                pPalette[i * 3 + 0] = R;
                pPalette[i * 3 + 1] = G;
                pPalette[i * 3 + 2] = B;
            }
        }
        break;
    case EPixelFormat::X8R8G8B8:
    case EPixelFormat::A8R8G8B8:
		{
			int32 const Count = ImagePitch / sizeof( uint32 );
			for (int32 i = 0; i < Count; ++i)
			{
				reinterpret_cast<uint32*>( &pData[0] )[i] = ARGB;
			}
		}
        break;
	case EPixelFormat::R32F:
		{
			int32 const Count = ImagePitch / sizeof( float32 );
			float32 const fR = (ARGB & 0x00ff0000u) * (1.0f / (1 << 16));
			for (int32 i = 0; i < Count; ++i)
			{
				reinterpret_cast<float32*>( &pData[0] )[i] = fR;
			}
		}
		break;
	case EPixelFormat::A32B32G32R32F:
		{
			int32 const Count = ImagePitch / (4 * sizeof( float32 ));
			float32 const fA = (ARGB & 0xff000000u) * (1.0f / (1 << 24));
			float32 const fR = (ARGB & 0x00ff0000u) * (1.0f / (1 << 16));
			float32 const fG = (ARGB & 0x0000ff00u) * (1.0f / (1 << 8));
			float32 const fB = (ARGB & 0x000000ffu) * (1.0f / (1 << 0));
			float32* pOut = reinterpret_cast<float32*>( &pData[0] );
			for (int32 i = 0; i < Count; ++i)
			{
				pOut[0] = fR;
				pOut[1] = fG;
				pOut[2] = fB;
				pOut[3] = fA;
				pOut += 4;
			}
		}
	case EPixelFormat::A16B16G16R16F:
		{
			int32 const Count = ImagePitch / (4 * sizeof( float16 ));
			float16 const fA = Float32ToFloat16((ARGB & 0xff000000u) * (1.0f / (1 << 24)) );
			float16 const fR = Float32ToFloat16((ARGB & 0x00ff0000u) * (1.0f / (1 << 16)) );
			float16 const fG = Float32ToFloat16((ARGB & 0x0000ff00u) * (1.0f / (1 << 8)) );
			float16 const fB = Float32ToFloat16( (ARGB & 0x000000ffu) * (1.0f / (1 << 0)) );
			float16* pOut = reinterpret_cast<float16*>( &pData[0] );
			for (int32 i = 0; i < Count; ++i)
			{
				pOut[0] = fR;
				pOut[1] = fG;
				pOut[2] = fB;
				pOut[3] = fA;
				pOut += 4;
			}
		}
        break;

	default:
		throw std::runtime_error( "unsupported format" );
    }
}

D3DFORMAT CImage::ConvertFormatToD3D( EPixelFormat::Type const& Format )
{
	switch (Format)
	{
		default:
		case EPixelFormat::None:
			return D3DFMT_UNKNOWN;
		case EPixelFormat::X8R8G8B8:
			return D3DFMT_X8R8G8B8;
		case EPixelFormat::A8R8G8B8:
			return D3DFMT_A8R8G8B8;
		case EPixelFormat::L8:
			return D3DFMT_L8;
		case EPixelFormat::R32F:
			return D3DFMT_R32F;
		case EPixelFormat::A32B32G32R32F:
			return D3DFMT_A32B32G32R32F;
		case EPixelFormat::A16B16G16R16F:
			return D3DFMT_A16B16G16R16F;
		case EPixelFormat::DXT1:
			return D3DFMT_DXT1;
		case EPixelFormat::DXT3:
			return D3DFMT_DXT3;
		case EPixelFormat::DXT5:
			return D3DFMT_DXT5;
	}
}

EPixelFormat::Type CImage::ConvertFormatFromD3D( D3DFORMAT Format )
{
	switch( Format )
	{
		default:
		case D3DFMT_UNKNOWN:
			return EPixelFormat::None;
			break;
		case D3DFMT_X8R8G8B8:
			return EPixelFormat::X8R8G8B8;
			break;
		case D3DFMT_A8R8G8B8:
			return EPixelFormat::A8R8G8B8;
			break;
		case D3DFMT_L8:
			return EPixelFormat::L8;
			break;
		case D3DFMT_R32F:
			return EPixelFormat::R32F;
			break;
		case D3DFMT_A32B32G32R32F:
			return EPixelFormat::A32B32G32R32F;
			break;
		case D3DFMT_A16B16G16R16F:
			return EPixelFormat::A16B16G16R16F;
			break;
		case D3DFMT_DXT1:
			return EPixelFormat::DXT1;
			break;
		case D3DFMT_DXT3:
			return EPixelFormat::DXT3;
			break;
		case D3DFMT_DXT5:
			return EPixelFormat::DXT5;
			break;
	}
}

LPDIRECT3DBASETEXTURE9 CImage::CreateD3DTexture( LPDIRECT3DDEVICE9 pDevice, DWORD Usage, 
    D3DFORMAT Format, D3DPOOL Pool, uint32 CreateFlags ) const
{
	//If an unknown format is requested, then automatically use the suggested one.
	if (Format == D3DFMT_UNKNOWN)
	{
		Format = ConvertFormatToD3D( m_Desc.Format );
	}

	bool bRaw = false;
	switch (m_Desc.Format)
	{
		case EPixelFormat::None:
			return NULL; //Not supported
		case EPixelFormat::DXT1:
			if (Format != D3DFMT_DXT1)
			{
				//Invalid translation
				return NULL;
			}
			bRaw = true;
			break;
		case EPixelFormat::DXT3:
			if (Format != D3DFMT_DXT3)
			{
				//Invalid translation
				return NULL;
			}
			bRaw = true;
			break;
		case EPixelFormat::DXT5:
			if (Format != D3DFMT_DXT5)
			{
				//Invalid translation
				return NULL;
			}
			bRaw = true;
			break;
	}

	int32 EffectiveWidth = m_Desc.Width;
	int32 EffectiveHeight = m_Desc.Height;
	if (bRaw)
	{
		EffectiveWidth = (m_Desc.Width + 3) & ~3;
		EffectiveHeight = (m_Desc.Height + 3) & ~3;
	}

	LPDIRECT3DTEXTURE9 hTexture = NULL;
	LPDIRECT3DVOLUMETEXTURE9 hVolumeTexture = NULL;
	LPDIRECT3DCUBETEXTURE9 hCubeTexture = NULL;
	LPDIRECT3DBASETEXTURE9 hBaseTexture = NULL;

	HRESULT hResult;

	if (CreateFlags & KCreateD3DTextureFlagsAsCubeMap)
	{
		if (m_Desc.Depth != 6 ||
			m_Desc.Width != m_Desc.Height )
		{
			throw std::invalid_argument( "image" );
		}
		hResult = pDevice->CreateCubeTexture( m_Desc.Width, 0, Usage, Format, Pool, &hCubeTexture, NULL );
		hBaseTexture = hCubeTexture;
	} else if (m_Desc.Depth == 1) {

		UINT Levels = 0;

		if (!Math_IsPow2( m_Desc.Width ) || !Math_IsPow2( m_Desc.Height )
#ifndef NDEBUG
			|| m_Desc.Width != m_Desc.Height
#endif
			)
		{
			D3DCAPS9 Caps;
			pDevice->GetDeviceCaps( &Caps );

			if (!Math_IsPow2( m_Desc.Width ) || !Math_IsPow2( m_Desc.Height ))
			{
				LOG_PURPLE( "Warning: non-power-of-two %dx%d texture not supported on all devices.", m_Desc.Width, m_Desc.Height );
				if (Caps.TextureCaps & D3DPTEXTURECAPS_POW2)
				{
					if (Caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)
					{
						LOG_RED( "Error: non-power-of-two %dx%d texture only conditionally supported on this device.", m_Desc.Width, m_Desc.Height );
						if (Usage & D3DUSAGE_AUTOGENMIPMAP)
						{
							LOG_RED( "       Disabling mipmapping. Automatic mipmapping is unusable. Also, manual mipmaps not provided yet (sorry).\n" );
							Usage &= ~D3DUSAGE_AUTOGENMIPMAP;
							Levels = 1;
						}
					} else {
						LOG_RED( "Error: non-power-of-two %dx%d texture not supported on this device.", m_Desc.Width, m_Desc.Height );
					}
				}
			}

			if (m_Desc.Width != m_Desc.Height)
			{
#if 0
				//Disable this warning - it was deemed to be too irritaiting
				//TODO consider re-enabling this.
				LOG_PURPLE( "Warning: non-square %dx%d texture not supported on all devices.", m_Desc.Width, m_Desc.Height );
#endif
				if (Caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)
				{
					LOG_RED( "Error: non-square %dx%d texture not supported on this device.", m_Desc.Width, m_Desc.Height );
				}
			}
		}

		hResult = pDevice->CreateTexture( m_Desc.Width, m_Desc.Height, Levels, 
			Usage, Format, Pool, &hTexture, NULL );
		hBaseTexture = hTexture;
	} else {
		hResult = pDevice->CreateVolumeTexture( m_Desc.Width, m_Desc.Height, m_Desc.Depth, 0 /*NumLevels*/,
			Usage, Format, Pool, &hVolumeTexture, NULL );
		hBaseTexture = hVolumeTexture;
	}

	if (FAILED( hResult ))
	{
		return NULL;
	}

	HImageLock hLock = const_cast<CImage*>(this)->Lock( EGraphicsResourceLock::ReadOnly );
	uint8 const* pSource = hLock->GetDataPtr();

	HImageLock hLockPal = const_cast<CImage*>(this)->LockPalette( EGraphicsResourceLock::ReadOnly );
	uint8 const* pSourcePal = hLockPal->GetDataPtr();

	uint8* pOutData = NULL;
	int32 OutPitch = 0;
	LPDIRECT3DSURFACE9 pSurface = NULL;
	LPDIRECT3DVOLUME9 pVolume = NULL;
	bool Result = true;
	if (hTexture != NULL)
	{
		hResult = hTexture->GetSurfaceLevel( 0, &pSurface );
		assert( SUCCEEDED( hResult ) );
		D3DLOCKED_RECT LockedRect;
		Result = SUCCEEDED( pSurface->LockRect( &LockedRect, NULL, 0 ) );
		OutPitch = LockedRect.Pitch;
		pOutData = static_cast<uint8*>( LockedRect.pBits );
	} else if ( hVolumeTexture != NULL ) {
		hResult = hVolumeTexture->GetVolumeLevel( 0, &pVolume );
		assert( SUCCEEDED( hResult ) );
		D3DLOCKED_BOX LockedBox;
		Result = SUCCEEDED( pVolume->LockBox( &LockedBox, NULL, 0 ) );
		OutPitch = LockedBox.SlicePitch;
		pOutData = static_cast<uint8*>( LockedBox.pBits );
	} else {
		assert( hCubeTexture != NULL );
		//Variables such as pOutData are set below for cube textures 
	}

	if (!Result)
	{
		//This may fail in some cases, e.g. a non-dynamic render target in video memory.
		return hBaseTexture;
	}

	std::vector<CVector4f> Buf;
	Buf.resize( EffectiveWidth );

	int32 k = 0;

	for (; k < m_Desc.Depth; ++k)
	{
		if (hCubeTexture != NULL)
		{
			HRESULT hResult = hCubeTexture->GetCubeMapSurface( static_cast<D3DCUBEMAP_FACES>( k ), 0, &pSurface );
			assert( SUCCEEDED( hResult ) );

			D3DLOCKED_RECT LockedRect;
			hResult = pSurface->LockRect( &LockedRect, NULL, 0 );
			if (FAILED( hResult ))
			{
				//This may fail in some cases, e.g. a non-dynamic render target in video memory.
				return hBaseTexture;
			}
			OutPitch = LockedRect.Pitch;
			pOutData = static_cast<uint8*>( LockedRect.pBits );
		}
		
		CVector4f* const pBuf = &Buf[0];

		for (int32 j = 0; j < EffectiveHeight; ++j)
		{
			switch (m_Desc.Format)
			{
			default:
				hBaseTexture = NULL;
				goto Done;

			case EPixelFormat::P8_R8G8B8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					uint32 const Index = pSource[i] * 3;
					pBuf[i].x = pSourcePal[Index];
					pBuf[i].y = pSourcePal[Index + 1];
					pBuf[i].z = pSourcePal[Index + 2];
					pBuf[i].w = 255;
				}
				break;

			case EPixelFormat::L8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					uint8 const L = pSource[i];
					pBuf[i].x = L;
					pBuf[i].y = L;
					pBuf[i].z = L;
					pBuf[i].w = 255;
				}
				break;

			case EPixelFormat::X8R8G8B8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					D3DCOLOR const SourceColor = reinterpret_cast<D3DCOLOR const*>( pSource )[i];
					pBuf[i].x = static_cast<float32>( (SourceColor >> 16) & 0xffu );
					pBuf[i].y = static_cast<float32>( (SourceColor >> 8) & 0xffu );
					pBuf[i].z = static_cast<float32>( SourceColor & 0xffu );
					pBuf[i].w = 255;
				}
				break;

			case EPixelFormat::A8R8G8B8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					D3DCOLOR const SourceColor = reinterpret_cast<D3DCOLOR const*>( pSource )[i];
					pBuf[i].x = static_cast<float32>( (SourceColor >> 16) & 0xffu );
					pBuf[i].y = static_cast<float32>( (SourceColor >> 8) & 0xffu );
					pBuf[i].z = static_cast<float32>( SourceColor & 0xffu );
					pBuf[i].w = static_cast<float32>( static_cast<uint32>( SourceColor ) >> 24 );
				}
				break;

			case EPixelFormat::R32F:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					pBuf[i].x = reinterpret_cast<float32 const*>( pSource )[i];
					pBuf[i].y = 255;
					pBuf[i].z = 255;
					pBuf[i].w = 255;
				}
				break;
			case EPixelFormat::A32B32G32R32F:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					pBuf[i].x = reinterpret_cast<float32 const*>( pSource )[4 * i + 0];
					pBuf[i].y = reinterpret_cast<float32 const*>( pSource )[4 * i + 1];
					pBuf[i].z = reinterpret_cast<float32 const*>( pSource )[4 * i + 2];
					pBuf[i].w = reinterpret_cast<float32 const*>( pSource )[4 * i + 3];
				}
				break;
			case EPixelFormat::A16B16G16R16F:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					pBuf[i].x = Float16ToFloat32( reinterpret_cast<float16 const*>( pSource )[4 * i + 0] );
					pBuf[i].y = Float16ToFloat32( reinterpret_cast<float16 const*>( pSource )[4 * i + 1] );
					pBuf[i].z = Float16ToFloat32( reinterpret_cast<float16 const*>( pSource )[4 * i + 2] );
					pBuf[i].w = Float16ToFloat32( reinterpret_cast<float16 const*>( pSource )[4 * i + 3] );
				}
				break;
			case EPixelFormat::DXT1:
				memcpy( pBuf, pSource, EffectiveWidth >> 1 );
				break;
			case EPixelFormat::DXT3:
			case EPixelFormat::DXT5:
				memcpy( pBuf, pSource, EffectiveWidth );
				break;
			}

			if (CreateFlags & KCreateD3DTextureFlagsAsOpacityMap)
			{
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					//Compute overall alpha value from luminance.
					//The pre-existing alpha is discarded.
					float32 const L = ComputeLuminance( pBuf[i].x, pBuf[i].y, pBuf[i].z );
					pBuf[i].w = L;
				}
			}

			switch (Format)
			{
			default:
				//Graphics format currently unsupported
				hBaseTexture = NULL;
				goto Done;

			case D3DFMT_A8R8G8B8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					D3DCOLOR& Color = reinterpret_cast<D3DCOLOR*>( pOutData )[i];
					int32 r = Float32ToInt32( pBuf[i].x );
					int32 g = Float32ToInt32( pBuf[i].y );
					int32 b = Float32ToInt32( pBuf[i].z );
					int32 a = Float32ToInt32( pBuf[i].w );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					if (a < 0) a = 0; else if (a > 255) a = 255;
					Color = (a << 24) + (r << 16) + (g << 8) + b;
				}
				break;

			case D3DFMT_X8R8G8B8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					D3DCOLOR& Color = reinterpret_cast<D3DCOLOR*>( pOutData )[i];
					int32 r = Float32ToInt32( pBuf[i].x );
					int32 g = Float32ToInt32( pBuf[i].y );
					int32 b = Float32ToInt32( pBuf[i].z );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					Color = 0xff000000u + (r << 16) + (g << 8) + b;
				}
				break;

			case D3DFMT_A8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					int32 a = Float32ToInt32( pBuf[i].w );
					if (a < 0) a = 0; else if (a > 255) a = 255;
					pOutData[i] = static_cast<uint8>( a );
				}
				break;

			case D3DFMT_L8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					int32 L = Float32ToInt32( ComputeLuminance( pBuf[i].x, pBuf[i].y, pBuf[i].z ) );
					if (L < 0) L = 0; else if (L > 255) L = 255;
					pOutData[i] = static_cast<uint8>( L );
				}
				break;

			case D3DFMT_A8L8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					uint16& Color = reinterpret_cast<uint16*>( pOutData )[i];
					int32 a = Float32ToInt32( pBuf[i].w );
					if (a < 0) a = 0; else if (a > 255) a = 255;
					int32 L = Float32ToInt32( ComputeLuminance( pBuf[i].x, pBuf[i].y, pBuf[i].z ) );
					if (L < 0) L = 0; else if (L > 255) L = 255;
					Color = static_cast<uint16>( (a << 8) + L );
				}
				break;

			case D3DFMT_L6V5U5:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					uint16& Color = reinterpret_cast<uint16*>( pOutData )[i];
					//Extract l from r, v from g, and u from b.
					int32 r = Float32ToInt32( pBuf[i].x );
					int32 g = Float32ToInt32( pBuf[i].y );
					int32 b = Float32ToInt32( pBuf[i].z );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					uint32 const l = ((r >> (24 - 16))                     ) & (((1 << 5) - 1) << 10);
					uint32 const v = ((g >> (16 - 10)) - (1 << (5 - 1 + 5))) & (((1 << 5) - 1) <<  5);
					uint32 const u = ((b >> (8  - 5 )) - (1 << (5 - 1 + 0))) & (((1 << 5) - 1) <<  0);
					Color = static_cast<uint16>( l + v + u );
				}
				break;

			case D3DFMT_X8L8V8U8:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					//Extract l from r, v from g, and u from b.
					int32 r = Float32ToInt32( pBuf[i].x );
					int32 g = Float32ToInt32( pBuf[i].y );
					int32 b = Float32ToInt32( pBuf[i].z );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					uint32 const l =  r << 16;
					uint32 const v = ((g - 0x80) & 0xff) << 8;
					uint32 const u = (b - 0x80) & 0xff;
					uint32& Color = reinterpret_cast<uint32*>( pOutData )[i];
					Color = l + v + u;
				}
				break;

			case D3DFMT_R32F:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					float32& Color = reinterpret_cast<float32*>( pOutData )[i];
					Color = pBuf[i].x;
				}
				break;

			case D3DFMT_A32B32G32R32F:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					float32* pColors = &reinterpret_cast<float32*>( pOutData )[4 * i];
					pColors[0] = pBuf[i].x;
					pColors[1] = pBuf[i].y;
					pColors[2] = pBuf[i].z;
					pColors[3] = pBuf[i].w;
				}
				break;

			case D3DFMT_A16B16G16R16F:
				for (int32 i = 0; i < m_Desc.Width; ++i)
				{
					float16* pColors = &reinterpret_cast<float16*>( pOutData )[4 * i];
					pColors[0] = Float32ToFloat16( pBuf[i].x );
					pColors[1] = Float32ToFloat16( pBuf[i].y );
					pColors[2] = Float32ToFloat16( pBuf[i].z );
					pColors[3] = Float32ToFloat16( pBuf[i].w );
				}
				break;
			
			case D3DFMT_DXT1:
				if (m_Desc.Format != EPixelFormat::DXT1)
				{
					//Conversion not supported
					hBaseTexture = NULL;
					goto Done;
				}

				//For DXT formats, LockRect() returns the pich for a block of bytes. We are processing
				//a row at a time. Set OutPitch to be the pitch of a row.
				OutPitch = EffectiveWidth >> 1; 
				memcpy( pOutData, pBuf, OutPitch );
				break;

			case D3DFMT_DXT3:
				if (m_Desc.Format != EPixelFormat::DXT3)
				{
					//Conversion not supported
					hBaseTexture = NULL;
					goto Done;
				}

				//For DXT formats, LockRect() returns the pich for a block of bytes. We are processing
				//a row at a time. Set OutPitch to be the pitch of a row.
				OutPitch = EffectiveWidth; 
				memcpy( pOutData, pBuf, OutPitch );
				break;

			case D3DFMT_DXT5:
				if (m_Desc.Format != EPixelFormat::DXT5)
				{
					//Conversion not supported
					hBaseTexture = NULL;
					goto Done;
				}

				//For DXT formats, LockRect() returns the pich for a block of bytes. We are processing
				//a row at a time. Set OutPitch to be the pitch of a row.
				OutPitch = EffectiveWidth; 
				memcpy( pOutData, pBuf, OutPitch );
				break;
			}

			//Advance to the next row
			pSource += m_Desc.RowPitch;
			pOutData += OutPitch;
		}

		if (hCubeTexture != NULL)
		{
			pSurface->UnlockRect();
			pSurface->Release();
		}
	}

	if (hTexture != NULL)
	{
		hBaseTexture = hTexture;
	} else if (hVolumeTexture != NULL) {
		hBaseTexture = hVolumeTexture;
	} else {
		assert( hCubeTexture != NULL );
		hBaseTexture = hCubeTexture;
	}

Done:
	Buf.clear();

	if (hTexture != NULL)
	{
		pSurface->UnlockRect();
		pSurface->Release();
	} else if (hVolumeTexture != NULL) {
		pVolume->UnlockBox();
		pVolume->Release();
	} else if (hCubeTexture != NULL && k <= D3DCUBEMAP_FACE_NEGATIVE_Z ) {
		pSurface->UnlockRect();
		pSurface->Release();
	}

	if (hBaseTexture != NULL)
	{
		//Update the texture's mip-maps
		hBaseTexture->GenerateMipSubLevels();
	}

	return hBaseTexture;
}

//TODO opacity maps should be applied in a preprocessing step
HImage WTCall ImageCombineRGBAlpha( CImage* pImage, CImage* pOpacityImage )
{
	if (pImage == NULL || pOpacityImage == NULL)
	{
		throw std::invalid_argument( "NULL image pointer" );
	}

	if (pImage->GetWidth() != pOpacityImage->GetWidth() ||
		pImage->GetHeight() != pOpacityImage->GetHeight() ||
		pImage->GetDepth() != pOpacityImage->GetDepth())
	{
		throw std::runtime_error( "incompatible image dimensions" );
	}

	int32 Width = pImage->GetWidth();
	int32 Height = pImage->GetHeight();
	int32 Depth = pImage->GetDepth();

	//Create a new image with A8R8G8B8 format
	CImage::Desc Desc;
	Desc.Width = Width;
	Desc.Height = Height;
	Desc.Depth = Depth;
	Desc.PaletteSize = 0;
	Desc.Format = EPixelFormat::A8R8G8B8;
	HImage hNewImage = CImage::CreateImage( Desc );

	for (int32 z = Depth - 1; z >= 0; --z)
	{
		for (int32 y = Height - 1; y >= 0; --y)
		{
			for (int32 x = Width - 1; x >= 0; --x)
			{
				//TODO optimize this loop
				uint32 Color = pImage->GetPixelColor( x, y, z );
				uint32 const OpacityColor = pOpacityImage->GetPixelColor( x, y, z );
				Color = (Color & 0x00ffffff) + (OpacityColor & 0xff000000);
				hNewImage->SetPixelColor( x, y, z, Color );
			}
		}
	}

	return hNewImage;
}

//TODO opacity maps should be applied in a preprocessing step
HImage WTCall ImageCombineRGBLuminance( CImage* pImage, CImage* pOpacityImage )
{
	if (pImage == NULL || pOpacityImage == NULL)
	{
		throw std::invalid_argument( "NULL image pointer" );
	}

	if (pImage->GetWidth() != pOpacityImage->GetWidth() ||
		pImage->GetHeight() != pOpacityImage->GetHeight() ||
		pImage->GetDepth() != pOpacityImage->GetDepth())
	{
		throw std::runtime_error( "incompatible image dimensions" );
	}

	int32 Width = pImage->GetWidth();
	int32 Height = pImage->GetHeight();
	int32 Depth = pImage->GetDepth();

	//Create a new image with A8R8G8B8 format
	CImage::Desc Desc;
	Desc.Width = Width;
	Desc.Height = Height;
	Desc.Depth = Depth;
	Desc.PaletteSize = 0;
	Desc.Format = EPixelFormat::A8R8G8B8;
	HImage hNewImage = CImage::CreateImage( Desc );

	for (int32 z = Depth - 1; z >= 0; --z)
	{
		for (int32 y = Height - 1; y >= 0; --y)
		{
			for (int32 x = Width - 1; x >= 0; --x)
			{
				//TODO optimize this loop

				//Compute alpha value from source luminance.
				uint32 const OpacityColor = pOpacityImage->GetPixelColor( x, y, z );
				uint32 const OpacityR = (OpacityColor >> 16) & 0xff;
				uint32 const OpacityG = (OpacityColor >>  8) & 0xff;
				uint32 const OpacityB = (OpacityColor      ) & 0xff;
				float32 const fOpacityA = ComputeLuminance( 
					static_cast<float32>( OpacityR ),
					static_cast<float32>( OpacityG ),
					static_cast<float32>( OpacityB ) );
				int32 OpacityA = Float32ToInt32( fOpacityA );
				assert( OpacityA >= 0 );
				assert( OpacityA <= 255 );

				//Combine the RGB and alpha.
				uint32 Color = pImage->GetPixelColor( x, y, z );
				Color = (Color & 0x00ffffff) | (OpacityA << 24);
				hNewImage->SetPixelColor( x, y, z, Color );
			}
		}
	}

	return hNewImage;
}

HImage WTCall CImage::CreateImage()
{
	return new CImage();
}

HImage WTCall CImage::CreateImage( CImage const& I )
{
	return new CImage( I );
}

HImage WTCall CImage::CreateImage( CImage::Desc const& Desc, uint32 ARGB )
{
	HImage hImage = new CImage();
    hImage->Init( Desc, ARGB );
	return hImage;
}

HImage WTCall CImage::CreateImage( System::IO::CStream* pStream, CImageSerializer* pSerializer )
{
	HImageSerializer hSerializer = pSerializer;

	if (hSerializer == NULL)
	{
		try
		{
			// Try autodetection
			for (;;)
			{
				hSerializer = new CImageSerializerDDS();
				if ( hSerializer->Validate(pStream ) )
				{
					break;
				}

				hSerializer = new CImageSerializerJPG();
				if ( hSerializer->Validate(pStream ) )
				{
					break;
				}

				hSerializer = new CImageSerializerPNG();
				if ( hSerializer->Validate(pStream ) )
				{
					break;
				}

				hSerializer = new CImageSerializerTGA();
				if ( hSerializer->Validate(pStream ) )
				{
					break;
				}

				hSerializer = new CImageSerializerWJ2();
				if ( hSerializer->Validate(pStream ) )
				{
					break;
				}

				return NULL;	// Failed to find a validating serializer
			}
		} catch (...) { //TODO catch only certain exception types
			return NULL;
		}
	}

	assert( hSerializer != NULL );
	HImage hImage = CreateImage();
	hSerializer->Decode( pStream, hImage );

	return hImage;
}

HImage WTCall CImage::CreateImage( string const& FileName, CImageSerializer* pSerializer )
{
	using namespace System::IO;

	HStream hStream = CFile::Open( FileName, FileMode::Open, 
		FileAccess::Read, FileShare::Read, FileFlags::KSequentialScan );
	if (hStream == NULL)
	{
		return NULL;
	}

	return CreateImage( hStream, pSerializer );
}

CImageSerializer* CreateImageSerializer( EImageFormat::Type ImageFormat )
{
	switch (ImageFormat)
	{
		case EImageFormat::Unknown:
			return NULL;

		case EImageFormat::DDS:
			return new CImageSerializerDDS();

		case EImageFormat::JPG:
			return new CImageSerializerJPG();

		case EImageFormat::PNG:
			return new CImageSerializerPNG();

		case EImageFormat::TGA:
			return new CImageSerializerTGA();

		case EImageFormat::WJ2:
			return new CImageSerializerWJ2();

		default:
			throw std::invalid_argument( "image format" );
	}
}

HImage WTCall CImage::CreateImage( System::IO::CStream * pStream, EImageFormat::Type ImageFormat )
{
    if (pStream == NULL)
    {
        return NULL;
    }

	HImageSerializer hSerializer = CreateImageSerializer( ImageFormat );
	return CreateImage( pStream, hSerializer );
}
	
HImage WTCall CImage::CreateImage( std::string const& FileName, EImageFormat::Type ImageFormat )
{
	using namespace System::IO;

	HStream hStream = CFile::Open( FileName, FileMode::Open, 
		FileAccess::Read, FileShare::Read, FileFlags::KSequentialScan );
	if (hStream == NULL)
	{
		return NULL;
	}

	return CreateImage( hStream, ImageFormat );
}

int32 CImage::Encode( System::IO::CStream* pStream, CImageSerializer* pSerializer ) const
{
    if (pStream == NULL || pSerializer == NULL)
    {
        return 0;
    }

	return pSerializer->Encode( pStream, this );
}

int32 CImage::Encode( std::string const& FileName, CImageSerializer* pSerializer ) const
{
    if (pSerializer == NULL)
    {
        return 0;
    }

	using namespace System::IO;

	HFileStream hStream = CFile::Open( FileName, FileMode::Create, 
		FileAccess::Write, FileShare::None, FileFlags::KSequentialScan );
	if (hStream == NULL)
	{
		return 0;
	}

	return pSerializer->Encode( hStream, this );
}

int32 CImage::Encode( System::IO::CStream* pStream, EImageFormat::Type ImageFormat ) const
{
	if (ImageFormat == EImageFormat::Unknown)
	{
		throw std::invalid_argument( "image encoding format" );
	}

	HImageSerializer hSerializer = CreateImageSerializer( ImageFormat );
	assert( hSerializer != NULL );
	return Encode( pStream, hSerializer );
}

int32 CImage::Encode( string const& FileName, EImageFormat::Type ImageFormat ) const
{
	using namespace System::IO;

	HFileStream hStream =  CFile::Open( FileName, FileMode::Create, 
		FileAccess::Write, FileShare::None, FileFlags::KSequentialScan );
	if (hStream == NULL)
	{
		return NULL;
	}

	return Encode( hStream, ImageFormat );
}

} //namespace Graphics
} //namespace WT
