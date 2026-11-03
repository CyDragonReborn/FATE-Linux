/***************************************************************************************
 *
 * ImageSerializerDDS.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 ***************************************************************************************/

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/stdafx.h"

#include <cassert>
#include "WT/Image/Image.h"
#include "dds.h"

using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS(CImageSerializerDDS)

/*virtual*/ bool CImageSerializerDDS::Validate( CStream* pStream ) const
{
    if (pStream == NULL)
    {
        return false;
    }

	struct
	{
		uint32 Magic;
		uint32 Size;
	} Header;

	int32 ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &Header ), sizeof Header );
	pStream->Seek( -ActualBytesRead, SeekOrigin::Current );

	if (ActualBytesRead != sizeof Header)
	{
        //Incomplete header
        return false;
	}

	if (Header.Magic != MAKEFOURCC( 'D', 'D', 'S', ' ' ) ||
		Header.Size != sizeof( DDS_HEADER )- sizeof Header.Magic)
	{
		//Not a DDS file
		return false;
	}

    return true;
}

/*virtual*/ void CImageSerializerDDS::Decode( CStream* pStream, CImage* pImage ) const
{
    if (pStream == NULL || pImage == NULL)
    {
		throw std::invalid_argument( "NULL pointer" );
    }

	DDS_HEADER ddsheader;
	int32 ActualSizeRetrieved = pStream->Read( reinterpret_cast<uint8*>( &ddsheader ), sizeof ddsheader );
	if (ActualSizeRetrieved != sizeof ddsheader)
	{
		throw std::invalid_argument( "bad DDS header" );
	}

	bool bBlocks = false;
	uint32 DestPixelPitch = sizeof( uint32 );
	CImage::Desc Desc;

	if (ddsheader.ddspf.dwFlags == (DDS_RGB | DDS_ALPHAPIXELS) ||
		ddsheader.ddspf.dwFlags == DDS_ALPHA)
	{
		Desc.Format = EPixelFormat::A8R8G8B8;
	} else if (ddsheader.ddspf.dwFlags == (DDS_LUMINANCE | DDS_ALPHAPIXELS)) {
		Desc.Format = EPixelFormat::A8R8G8B8;
		ddsheader.ddspf.dwGBitMask = ddsheader.ddspf.dwRBitMask;
		ddsheader.ddspf.dwBBitMask = ddsheader.ddspf.dwRBitMask;
	} else if (ddsheader.ddspf.dwFlags == DDS_RGB ||
			   ddsheader.ddspf.dwFlags == DDS_BUMPDUDV) {
		Desc.Format = EPixelFormat::X8R8G8B8;
	} else if (ddsheader.ddspf.dwFlags == DDS_LUMINANCE) {
		if (ddsheader.ddspf.dwRGBBitCount != 8)
		{
			throw std::runtime_error( "DDS luminance bit count" );
		}

		Desc.Format = EPixelFormat::L8;
		DestPixelPitch = sizeof( uint8 );
	} else if (ddsheader.ddspf.dwFlags == DDS_FOURCC) {

		if (ddsheader.ddspf.dwFourCC == MAKEFOURCC( 0x75, 0, 0, 0 ))
		{
			//CxV8U8 format

			//Require nonzero bit information
			if (ddsheader.ddspf.dwRGBBitCount != 16)
			{
				throw std::runtime_error( "DDS CxV8U8 bit count" );
			}

			Desc.Format = EPixelFormat::X8R8G8B8;
		} else if (ddsheader.ddspf.dwFourCC == MAKEFOURCC( 0x72, 0, 0, 0)) {
			Desc.Format = EPixelFormat::R32F;
			ddsheader.ddspf.dwRGBBitCount = 32;
			DestPixelPitch = sizeof( float32 );
		} else if (ddsheader.ddspf.dwFourCC == MAKEFOURCC( 0x3f, 0, 0, 0)) {
			//Q8W8V8U8 format
			ddsheader.ddspf.dwRGBBitCount = 32;
			ddsheader.ddspf.dwRBitMask = 0x00ff0000;
			ddsheader.ddspf.dwGBitMask = 0x0000ff00;
			ddsheader.ddspf.dwBBitMask = 0x000000ff;
			ddsheader.ddspf.dwABitMask = 0xff000000;

			Desc.Format = EPixelFormat::A8R8G8B8;
		} else if (ddsheader.ddspf.dwFourCC == MAKEFOURCC( 0x74, 0, 0, 0)) {
			//A32B32G32R32F format
			Desc.Format = EPixelFormat::A32B32G32R32F;
			ddsheader.ddspf.dwRGBBitCount = 128;
			DestPixelPitch = 4 * sizeof( float32 );
		} else if (ddsheader.ddspf.dwFourCC == MAKEFOURCC( 0x71, 0, 0, 0)) {
			Desc.Format = EPixelFormat::A16B16G16R16F;
			ddsheader.ddspf.dwRGBBitCount = 64;
			DestPixelPitch = 4 * sizeof( float16 );
		} else if (ddsheader.ddspf == DDSPF_DXT1) {
			bBlocks = true;
			Desc.Format = EPixelFormat::DXT1;
			ddsheader.ddspf.dwRGBBitCount = 4; //0.5 bytes per pixel
		} else if (ddsheader.ddspf == DDSPF_DXT3) {
			bBlocks = true;
			Desc.Format = EPixelFormat::DXT3;
			ddsheader.ddspf.dwRGBBitCount = 8; //1 byte per pixel
		} else if (ddsheader.ddspf == DDSPF_DXT5) {
			bBlocks = true;
			Desc.Format = EPixelFormat::DXT5;
			ddsheader.ddspf.dwRGBBitCount = 8; //1 byte per pixel
		} else {
			throw std::runtime_error( "unsupported DDS fourCC" );
		}
	} else {
		throw std::runtime_error( "unsupported DDS flags" );
	}

	if (!bBlocks &&
		Desc.Format != EPixelFormat::R32F &&
		Desc.Format != EPixelFormat::A32B32G32R32F &&
		Desc.Format != EPixelFormat::A16B16G16R16F &&
		(ddsheader.ddspf.dwRGBBitCount & 7 ||
		 ddsheader.ddspf.dwRGBBitCount == 0 ||
		 ddsheader.ddspf.dwRGBBitCount > 32))
	{
		throw std::runtime_error( "unsupported DDS format" );
	}

	Desc.Width = ddsheader.dwWidth;
	Desc.Height = ddsheader.dwHeight;
	Desc.Depth = 1;
	
	bool bCubeMap = false;
	if (ddsheader.dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME)
	{
		Desc.Depth = ddsheader.dwDepth;
	} else if (ddsheader.dwCubemapFlags & DDS_CUBEMAP) {
		//If any cube map face is defined, then we are loading a cube map.
		bCubeMap = true;
		Desc.Depth = 6;
	}

	int32 SourcePixelPitch = ddsheader.ddspf.dwRGBBitCount >> 3;
	int32 SourceRowPitch, SourceSlicePitch;
	int32 EffectiveHeight = Desc.Height;

	if (bBlocks)
	{
		EffectiveHeight = (Desc.Height + 3) & ~3;
		SourceRowPitch = (((Desc.Width + 3) & ~3) * ddsheader.ddspf.dwRGBBitCount) >> 3;
	} else {
		SourceRowPitch = Desc.Width * SourcePixelPitch;
	}
	SourceSlicePitch = EffectiveHeight * SourceRowPitch;

	uint32 MipMapCount = 0;
	if (ddsheader.dwHeaderFlags & DDS_HEADER_FLAGS_MIPMAP)
	{
		MipMapCount = ddsheader.dwMipMapCount;
	}

	uint32 MipMapSize = 0;
	uint32 Width = Desc.Width;
	uint32 Height = Desc.Height;
	for (uint32 i = 1; i < MipMapCount; ++i)
	{
		if (!(Width >>= 1)) {Width = 1;}
		if (!(Height >>= 1)) {Height = 1;}

		if (bBlocks)
		{
			MipMapSize += (((Width + 3) & ~3) * ((Height + 3) & ~3) * ddsheader.ddspf.dwRGBBitCount) >> 3;
		} else {
			MipMapSize += Width * Height * SourcePixelPitch;
		}
	}

	Desc.PaletteSize = 0;

    //If failed to initialize image, give up
	pImage->Init( Desc );

    //Read the description with updated variables (e.g. row pitch)
    Desc = pImage->GetDesc();

	std::vector<uint8> Source;
	Source.resize( SourceSlicePitch );

    HImageLock hLock = pImage->Lock( EGraphicsResourceLock::WriteOnly );
	uint8 *pDest = hLock->GetDataPtr();
   
	uint8* pDestSlice = pDest;
	for (int32 i = 0; i < Desc.Depth; ++i, pDestSlice += Desc.SlicePitch)
	{
		if (bCubeMap)
		{
			//This face does not exist in the file; if so simply fill the slice
			//with a white color and continue.
			if (!(ddsheader.dwCubemapFlags & (DDS_CUBEMAP_POSITIVEX << i)))
			{
				memset( pDest, 0xff, Desc.SlicePitch );
				continue;
			}
		}

		ActualSizeRetrieved = pStream->Read( reinterpret_cast<uint8*>( &Source[0] ), SourceSlicePitch );

		if (ActualSizeRetrieved != SourceSlicePitch)
		{
			throw std::runtime_error( "truncated DDS stream" );
		}

		if (bCubeMap)
		{
			//We need to skip over mip maps in the file here to get to the next face (if any).
			if (MipMapSize)
			{
				uint32 ActualBytesSkipped;
				ActualBytesSkipped = pStream->Seek( MipMapSize, SeekOrigin::Current );
				if (ActualBytesSkipped != MipMapSize)
				{
					throw std::runtime_error( "truncated DDS stream " );
				}
			}
		}

		uint8 const* pSourceRow = &Source[0];
		uint8* pDestRow = pDestSlice;

		for (int32 j = 0; j < EffectiveHeight; ++j)
		{
			uint8 const* pSourcePixel = pSourceRow;
			uint8* pDestPixel = pDestRow;

			if (Desc.Format == EPixelFormat::DXT1 ||
				Desc.Format == EPixelFormat::DXT3 ||
				Desc.Format == EPixelFormat::DXT5)
			{
				memcpy( pDestPixel, pSourcePixel, SourceRowPitch );
			} else if (Desc.Format == EPixelFormat::R32F) {
				for (int32 k = 0; k < Desc.Width; ++k)
				{
					float32 const& SourcePixel = *reinterpret_cast<float32 const*>( pSourcePixel );
					float32& DestPixel = *reinterpret_cast<float32*>( pDestPixel );
					DestPixel = SourcePixel;
					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			} else if (Desc.Format == EPixelFormat::A32B32G32R32F) {
				for (int32 k = 0; k < Desc.Width; ++k)
				{
					reinterpret_cast<float32*>( pDestPixel )[0] = reinterpret_cast<float32 const*>( pSourcePixel )[0];
					reinterpret_cast<float32*>( pDestPixel )[1] = reinterpret_cast<float32 const*>( pSourcePixel )[1];
					reinterpret_cast<float32*>( pDestPixel )[2] = reinterpret_cast<float32 const*>( pSourcePixel )[2];
					reinterpret_cast<float32*>( pDestPixel )[3] = reinterpret_cast<float32 const*>( pSourcePixel )[3];
					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			} else if (Desc.Format == EPixelFormat::A16B16G16R16F) {
				for (int32 k = 0; k < Desc.Width; ++k)
				{
					reinterpret_cast<float16*>( pDestPixel )[0] = reinterpret_cast<float16 const*>( pSourcePixel )[0];
					reinterpret_cast<float16*>( pDestPixel )[1] = reinterpret_cast<float16 const*>( pSourcePixel )[1];
					reinterpret_cast<float16*>( pDestPixel )[2] = reinterpret_cast<float16 const*>( pSourcePixel )[2];
					reinterpret_cast<float16*>( pDestPixel )[3] = reinterpret_cast<float16 const*>( pSourcePixel )[3];
					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			} else if (DestPixelPitch == sizeof( uint8 )) {
				for (int32 k = 0; k < Desc.Width; ++k)
				{
					*pDestPixel++ = *pSourcePixel++;
				}
			} else if (ddsheader.ddspf.dwRGBBitCount == 32) {
				for (int32 k = 0; k < Desc.Width; ++k)
				{
					uint32 const& SourcePixel = *reinterpret_cast<uint32 const*>( pSourcePixel );
					uint32& DestPixel = *reinterpret_cast<uint32*>( pDestPixel );

					DestPixel = SourcePixel;

					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			} else if (ddsheader.ddspf.dwRGBBitCount == 24) {
				for (int32 k = 0; k < Desc.Width; ++k)
				{

					uint32 const SourcePixel = 
						static_cast<uint32>( pSourcePixel[0] ) +
						(static_cast<uint32>( pSourcePixel[1] ) << 8) + 
						(static_cast<uint32>( pSourcePixel[2] ) << 16) +
						0xff000000u;

					uint32& DestPixel = *reinterpret_cast<uint32*>( pDestPixel );

					DestPixel = SourcePixel;

					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			} else if (ddsheader.ddspf.dwRGBBitCount == 16) {

				float32 const AFactor = ddsheader.ddspf.dwABitMask ? 255.0f / ddsheader.ddspf.dwABitMask : 0.0f;
				float32 const RFactor = ddsheader.ddspf.dwRBitMask ? 255.0f / ddsheader.ddspf.dwRBitMask : 0.0f;
				float32 const GFactor = ddsheader.ddspf.dwGBitMask ? 255.0f / ddsheader.ddspf.dwGBitMask : 0.0f;
				float32 const BFactor = ddsheader.ddspf.dwBBitMask ? 255.0f / ddsheader.ddspf.dwBBitMask : 0.0f;
				//Set components to 255 when no mask is present
				float32 const ATerm = 255.0f - ddsheader.ddspf.dwABitMask * AFactor;
				float32 const RTerm = 255.0f - ddsheader.ddspf.dwRBitMask * RFactor;
				float32 const GTerm = 255.0f - ddsheader.ddspf.dwGBitMask * GFactor;
				float32 const BTerm = 255.0f - ddsheader.ddspf.dwBBitMask * BFactor;
					
				for (int32 k = 0; k < Desc.Width; ++k)
				{
					uint16 const& SourcePixel = *reinterpret_cast<uint16 const*>( pSourcePixel );
					uint32& DestPixel = *reinterpret_cast<uint32*>( pDestPixel );

					uint32 const a = Float32ToInt32( AFactor * (SourcePixel & ddsheader.ddspf.dwABitMask) + ATerm );
					uint32 const r = Float32ToInt32( RFactor * (SourcePixel & ddsheader.ddspf.dwRBitMask) + RTerm );
					uint32 const g = Float32ToInt32( GFactor * (SourcePixel & ddsheader.ddspf.dwGBitMask) + GTerm );
					uint32 const b = Float32ToInt32( BFactor * (SourcePixel & ddsheader.ddspf.dwBBitMask) + BTerm );

					DestPixel = (a << 24) | (r << 16) | (g << 8) | b;

					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			} else {
				assert( ddsheader.ddspf.dwABitMask == 0xff );

				for (int32 k = 0; k < Desc.Width; ++k)
				{
					uint32 const& SourcePixel = *reinterpret_cast<uint8 const*>( pSourcePixel );
					uint32& DestPixel = *reinterpret_cast<uint32*>( pDestPixel );

					DestPixel = (SourcePixel << 24) | 0x00ffffff;

					pSourcePixel += SourcePixelPitch;
					pDestPixel += DestPixelPitch;
				}
			}

			pSourceRow += SourceRowPitch;
			pDestRow += Desc.RowPitch;
		}
	}
}

/*virtual*/ int32 CImageSerializerDDS::Encode( CStream* pStream, CImage const* pImage ) const
{
    //If image or stream pointers are NULL, then fail
    if (pImage == NULL || pStream == NULL)
    {
        return 0;
    }
    
    //If image is locked then fail
    if (pImage->IsLocked() || pImage->IsPaletteLocked())
    {
        return 0;
    }

	CImage::Desc Desc = pImage->GetDesc();

	//Check image format
	switch (Desc.Format)
	{
		case EPixelFormat::A8R8G8B8:
		case EPixelFormat::X8R8G8B8:
		case EPixelFormat::L8:
		case EPixelFormat::R32F:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
		case EPixelFormat::DXT1:
		case EPixelFormat::DXT3:
		case EPixelFormat::DXT5:
			break;
		default:
			return 0; //Not supported
	}

    //Lock image data
    HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
    uint8 *pSource = hLock->GetDataPtr();
	
    uint32 OrigPosition = pStream->GetPosition();

	DDS_HEADER ddsheader;
	memset( &ddsheader, 0, sizeof ddsheader );
	ddsheader.dwMagic = MAKEFOURCC( 'D', 'D', 'S', ' ' );
	ddsheader.dwSize = sizeof ddsheader - sizeof ddsheader.dwMagic;
	ddsheader.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE; //| DDS_HEADER_FLAGS_PITCH;
	ddsheader.dwHeight = Desc.Height;
	ddsheader.dwWidth = Desc.Width;
	ddsheader.dwMipMapCount = 0;
	ddsheader.ddspf.dwSize = sizeof ddsheader.ddspf;

	bool bBlocks = false;
	switch (Desc.Format)
	{
	default:
		assert( false );
	case EPixelFormat::A8R8G8B8:
		ddsheader.ddspf.dwFlags = DDS_RGB | DDS_ALPHAPIXELS;
		ddsheader.ddspf.dwRGBBitCount = 32;
		ddsheader.ddspf.dwRBitMask = 0x00ff0000;
		ddsheader.ddspf.dwGBitMask = 0x0000ff00;
		ddsheader.ddspf.dwBBitMask = 0x000000ff;
		ddsheader.ddspf.dwABitMask = 0xff000000;
		break;
	case EPixelFormat::X8R8G8B8:
		ddsheader.ddspf.dwFlags = DDS_RGB;
		ddsheader.ddspf.dwRGBBitCount = 24;
		ddsheader.ddspf.dwRBitMask = 0x00ff0000;
		ddsheader.ddspf.dwGBitMask = 0x0000ff00;
		ddsheader.ddspf.dwBBitMask = 0x000000ff;
		break;
	case EPixelFormat::L8:
		ddsheader.ddspf.dwFlags = DDS_LUMINANCE;
		ddsheader.ddspf.dwRGBBitCount = 8;
		ddsheader.ddspf.dwRBitMask = 0x000000ff;
		break;
	case EPixelFormat::R32F:
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
		ddsheader.dwPitchOrLinearSize = ddsheader.dwWidth * ddsheader.dwHeight * sizeof( float32 );
		ddsheader.ddspf.dwFlags = DDS_FOURCC;
		ddsheader.ddspf.dwFourCC = MAKEFOURCC( 0x72, 0, 0, 0);
		break;
	case EPixelFormat::A32B32G32R32F:
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
		ddsheader.dwPitchOrLinearSize = ddsheader.dwWidth * ddsheader.dwHeight * sizeof( float32 ) * 4;
		ddsheader.ddspf.dwFlags = DDS_FOURCC;
		ddsheader.ddspf.dwFourCC = MAKEFOURCC( 0x74, 0, 0, 0);
		break;
	case EPixelFormat::A16B16G16R16F:
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
		ddsheader.dwPitchOrLinearSize = ddsheader.dwWidth * ddsheader.dwHeight * sizeof( float16 ) * 4;
		ddsheader.ddspf.dwFlags = DDS_FOURCC;
		ddsheader.ddspf.dwFourCC = MAKEFOURCC( 0x71, 0, 0, 0);
		break;
	case EPixelFormat::DXT1:
		bBlocks = true;
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
		ddsheader.dwPitchOrLinearSize = ((ddsheader.dwWidth + 3) & ~3) * ((ddsheader.dwHeight + 3) & ~3) >> 1;
		ddsheader.ddspf = DDSPF_DXT1;
		break;
	case EPixelFormat::DXT3:
		bBlocks = true;
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
		ddsheader.dwPitchOrLinearSize = ((ddsheader.dwWidth + 3) & ~3) * ((ddsheader.dwHeight + 3) & ~3);
		ddsheader.ddspf = DDSPF_DXT3;
		break;
	case EPixelFormat::DXT5:
		bBlocks = true;
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
		ddsheader.dwPitchOrLinearSize = ((ddsheader.dwWidth + 3) & ~3) * ((ddsheader.dwHeight + 3) & ~3);
		ddsheader.ddspf = DDSPF_DXT3;
		break;
	}

	if (ddsheader.dwDepth != 1)
	{
		ddsheader.dwDepth = Desc.Depth;
		ddsheader.dwHeaderFlags |= DDS_HEADER_FLAGS_VOLUME;
	}

	int32 EffectiveWidth = ddsheader.dwWidth;
	int32 EffectiveHeight = ddsheader.dwHeight;
	int32 EffectiveBits = ddsheader.ddspf.dwRGBBitCount;

	if (bBlocks)
	{
		EffectiveWidth = (ddsheader.dwWidth + 3) & ~3;
		EffectiveHeight = (ddsheader.dwHeight + 3) & ~3;
		EffectiveBits = 8;
		if (Desc.Format == EPixelFormat::DXT1)
		{
			EffectiveBits = 4;
		}
	}

	uint32 const RowPitch = (EffectiveWidth * EffectiveBits) >> 3;
	
	//Set dwPitchOrLinearSize if not yet set
	if (ddsheader.dwPitchOrLinearSize == 0)
	{
		ddsheader.dwPitchOrLinearSize = RowPitch;
	}

	pStream->Write( reinterpret_cast<uint8*>( &ddsheader ), sizeof ddsheader );

	std::vector<uint8> Dest;
	Dest.resize( RowPitch );
	//memset( pDest, 0, RowPitch );

	uint8 const* pSourceSlice = pSource;
	for (int32 i = 0; i < Desc.Depth; ++i, pSourceSlice += Desc.SlicePitch)
	{
		uint8 const* pSourceRow = pSourceSlice;
		for (int32 j = 0; j < Desc.Height; ++j, pSourceRow += Desc.RowPitch)
		{
			uint8 const* pSourcePixel = pSourceRow;
			uint8* pDestPixel = &Dest[0];
				
			switch (Desc.Format)
			{
				default:
					assert( false );

				case EPixelFormat::DXT1:
				case EPixelFormat::DXT3:
				case EPixelFormat::DXT5:
					memcpy( pDestPixel, pSourcePixel, Desc.SlicePitch );
					break;

				case EPixelFormat::A8R8G8B8:
					for (int32 k = 0; k < Desc.Width; ++k, pSourcePixel += 4, pDestPixel += 4 )
					{
						uint32 const SourcePixel = *reinterpret_cast<uint32 const*>( pSourcePixel );
						*reinterpret_cast<uint32*>( pDestPixel ) = SourcePixel;
					}
					break;

				case EPixelFormat::X8R8G8B8:
					for (int32 k = 0; k < Desc.Width; ++k, pSourcePixel += 4, pDestPixel += 3 )
					{
						uint32 const SourcePixel = *reinterpret_cast<uint32 const*>( pSourcePixel );
						pDestPixel[0] = static_cast<uint8>( SourcePixel );
						pDestPixel[1] = static_cast<uint8>( SourcePixel >> 8 );
						pDestPixel[2] = static_cast<uint8>( SourcePixel >> 16 );
					}
					break;

				case EPixelFormat::L8:
					for (int32 k = 0; k < Desc.Width; ++k)
					{
						pDestPixel[k] = pSourcePixel[k];
					}
					break;

				case EPixelFormat::R32F:
					for (int32 k = 0; k < Desc.Width; ++k, pSourcePixel += 4, pDestPixel += 4)
					{
						float32 const SourcePixel = *reinterpret_cast<float32 const*>( pSourcePixel );
						*reinterpret_cast<float32*>( pDestPixel ) = SourcePixel;
					}
					break;

				case EPixelFormat::A32B32G32R32F:
					for (int32 k = 0; k < 4 * Desc.Width; ++k, pSourcePixel += 4, pDestPixel += 4)
					{
						float32 const SourcePixel = *reinterpret_cast<float32 const*>( pSourcePixel );
						*reinterpret_cast<float32*>( pDestPixel ) = SourcePixel;
					}
					break;

				case EPixelFormat::A16B16G16R16F:
					for (int32 k = 0; k < 4 * Desc.Width; ++k, pSourcePixel += 2, pDestPixel += 2)
					{
						float16 const SourcePixel = *reinterpret_cast<float16 const*>( pSourcePixel );
						*reinterpret_cast<float16*>( pDestPixel ) = SourcePixel;
					}
					break;
			}

			pStream->Write( reinterpret_cast<uint8*>( &Dest[0] ), RowPitch );
		}
	}

    return pStream->GetPosition() - OrigPosition;
}

} //namespace Graphics
} //namespace WT
