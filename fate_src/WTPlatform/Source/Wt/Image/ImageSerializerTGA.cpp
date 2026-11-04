/***************************************************************************************
 *
 * ImageSerializerTGA.h
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 6/29/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------

#include "WT/stdafx.h"

#include <cassert>
#include "WT/Image/Image.h"

using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CImageSerializerTGA )

//Raw TGA header
#pragma pack(push)
#pragma pack(1)
struct TGAHeaderRaw
{
    uint8 numberofcharactersinidentificationfield;
    uint8 colormaptype;
    uint8 imagetype;
    int16 colormaporigin;
    int16 colormaplength;
    int8  colormapbits;
    int16 xorigin;
    int16 yorigin;
    int16 width;
    int16 height;
    int8  imagepixelbits;
    int8  imagedescriptorbyte;
};
#pragma pack(pop)

//Easy-to-use TGA header class
class CTGAHeader
{
public:
    int16 m_Width;
	int16 m_Height;
    int32 m_PixelBits;
    int32 m_AlphaBits;
    int32 m_ColorMapLength;
    int32 m_ColorMapBits;
    bool m_bFlipHorizontal;
    bool m_bFlipVertical;

    bool Decode( CStream* pStream );
    bool Encode( CStream* pStream );
    static bool Validate( TGAHeaderRaw const& Raw );
};

bool CTGAHeader::Validate(TGAHeaderRaw const& Raw)
{
    if (Raw.colormaptype != 0 && Raw.colormaptype != 1) 
    {
        return false; //Invalid color map type
    }

    /*
    Valid TGA image types:

    0  -  No image data included.
    1  -  Uncompressed, color-mapped images.
    2  -  Uncompressed, RGB images.
    3  -  Uncompressed, black and white images.
    9  -  Runlength encoded color-mapped images.
   10  -  Runlength encoded RGB images.
   11  -  Compressed, black and white images.
   32  -  Compressed color-mapped data, using Huffman, Delta, and
          runlength encoding.
   33  -  Compressed color-mapped data, using Huffman, Delta, and
          runlength encoding.  4-pass quadtree-type process.
    */

    if (Raw.imagetype > 3) //Does the image use compression?
    {
        return false; //Compression not supported
    }

    if (Raw.imagetype == 0) //Is there image data?
    {
        return false; //"none" image type not supported
    }

    if (Raw.colormaptype)
    {
        switch (Raw.colormapbits)
        {
        default:
            return false; //Invalid color map bits
        case 16:
        case 24:
        case 32:
            ;
        }
    }

    switch (Raw.imagepixelbits)
    {
    default:
        return false; //Invalid image pixel bits
    case 8:
    case 16:
    case 24:
    case 32:
        ;
    }

    //If palettized, check the number of index bits
    if (Raw.imagetype == 1 && Raw.imagepixelbits > 16)
    {
        return false; //Only 1- or 2-byte palette indices allowed
    }

    uint8 alphabits = Raw.imagedescriptorbyte & 0xf;

    //Based on the number of bits per pixel (not index),
    //the number of alpha bits must be a certain value
    switch (Raw.imagetype == 1 ? 
        Raw.colormapbits : Raw.imagepixelbits)
    {
    case 8:
        if (alphabits != 8)
        {
            return false; //8-bit must have 8 bits of attribute
        }
        break;
    case 16: 
        if (alphabits > 1)
        {
            return false; //16-bit must have 0 or 1 bits of alpha
        }
        break;
    case 24:
#if 0   //Don't check this. Accept non-conforming TGA headers produced by Photoshop 7.
        if (alphabits != 0)
        {
            return false; //24-bit must not have alpha
        }
#endif
        break;
    case 32:
        if (alphabits != 8)
        {
            return false; //32-bit pixels must have 8-bit alpha
        }
        break;
    }

    if ((Raw.imagedescriptorbyte & 0xc0) != 0)
    {
        return false; //Interleaving not supported
    }

    return true;
}


bool CTGAHeader::Decode(CStream* pStream)
{
    TGAHeaderRaw Raw;

    uint32 ActualSizeRetrieved = pStream->Read( reinterpret_cast<uint8*>( &Raw ), sizeof Raw );
    if (ActualSizeRetrieved != sizeof Raw)
    {
        //Incomplete header
        return false;
    }

    bool Result = Validate(Raw);
    if (!Result)
    {
        //Invalid header
        return Result;
    }

    uint8 alphabits = Raw.imagedescriptorbyte & 0xf;

    //Accept non-conforming TGA headers produced by Photoshop 7.
    //If the image is 24-bit color, then set attribute bits to 0.
    if ((Raw.imagetype == 1 ? Raw.colormapbits : Raw.imagepixelbits) == 24)
    {
        alphabits = 0;
    }

    //If no palette is present, discard information
    if (!Raw.colormaptype)
    {
        Raw.colormaplength = 0;
        Raw.colormapbits = 0;
    }

    //Skip over color map if it exists but is not used
    if (Raw.colormaptype && Raw.colormaplength && Raw.imagetype != 1)
    {
        int32 colormapbytes = (Raw.colormapbits + 7) >> 3;
        {
            uint32 BytesToSkip = Raw.colormaplength * colormapbytes;
			pStream->Seek( BytesToSkip, SeekOrigin::Current );
        }
        Raw.colormaplength = 0;
        Raw.colormapbits = 0;
    }

    m_Width = Raw.width;
    m_Height = Raw.height;
    m_PixelBits = Raw.imagepixelbits;
    m_AlphaBits = alphabits;
    m_ColorMapLength = Raw.colormaplength;
    m_ColorMapBits = Raw.colormapbits;
    m_bFlipHorizontal = Raw.imagedescriptorbyte & 0x10 ? true : false;
    m_bFlipVertical = Raw.imagedescriptorbyte & 0x20 ? false : true;

    {
        uint32 BytesToSkip = Raw.numberofcharactersinidentificationfield;
		pStream->Seek( BytesToSkip, SeekOrigin::Current );
    }

    //Ready to read color map data and image data at this point

	return true;
}

bool CTGAHeader::Encode(CStream* pStream)
{
    uint8 imagetype;
    uint8 imagedescriptorbyte = static_cast<uint8>(m_AlphaBits);
    
    if (m_ColorMapLength)
    {
        imagetype = 1;
    } else {
        switch (m_PixelBits)
        {
        case 8:
            imagetype = 3; //Grayscale
            break;
        case 24:
        case 32:
            imagetype = 2; //RGB
            break;
        default:
            return false;
        }
    }

    if (m_bFlipHorizontal)
        imagedescriptorbyte |= 0x10;

    if (!m_bFlipVertical)
        imagedescriptorbyte |= 0x20;

    TGAHeaderRaw Raw;
    Raw.numberofcharactersinidentificationfield = 0;
    Raw.colormaptype = m_ColorMapLength ? 1 : 0;
    Raw.imagetype = imagetype;
    Raw.colormaporigin = 0;
    Raw.colormaplength = static_cast<int16>(m_ColorMapLength);
    Raw.colormapbits = static_cast<int8>(m_ColorMapBits);
    Raw.xorigin = 0;
    Raw.yorigin = 0;
    Raw.width = m_Width;
    Raw.height = m_Height;
    Raw.imagepixelbits = static_cast<int8>(m_PixelBits);
    Raw.imagedescriptorbyte = imagedescriptorbyte;

    pStream->Write( reinterpret_cast<uint8*>( &Raw ), sizeof Raw);

    //Write no identification field

    //The color map data (if any) and image data needs to be written 
    //outside of this function.

	return true;
}

/*virtual*/ bool CImageSerializerTGA::Validate( CStream* pStream ) const
{
    if (pStream == NULL)
    {
        return false;
    }

    bool Result = false;
    try
    {
        TGAHeaderRaw Raw;

        int32 ActualBytesRead = static_cast<int32>( pStream->Read( reinterpret_cast<uint8*>( &Raw ), sizeof Raw ) );
		pStream->Seek( -ActualBytesRead, SeekOrigin::Current );

        if (ActualBytesRead != sizeof Raw)
        {
            //Incomplete header
            return false;
        }

        if (CTGAHeader::Validate( Raw ))
        {
            Result = true;
        }
    } catch (...) {
    }

    return Result;
}

/*virtual*/ void CImageSerializerTGA::Decode( CStream* pStream, CImage* pImage ) const
{
    if (pStream == NULL)
    {
		throw std::invalid_argument( "NULL stream" );
    }

    if (pImage == NULL)
    {
		throw std::invalid_argument( "NULL image" );
    }

    CTGAHeader Header;

	std::vector<uint8> ColorMap;

    if (!Header.Decode(pStream))
    {
		throw std::runtime_error( "bad TGA header" );
    }

    CImage::Desc Desc;

    if (Header.m_ColorMapLength)
    {
        switch (Header.m_ColorMapBits)
        {
        case 16:
        case 32: 
            Desc.Format = EPixelFormat::A8R8G8B8; 
            break;
        case 24: 
            Desc.Format = EPixelFormat::X8R8G8B8; 
            break;
        default:
            assert( 0 );
        }

        //Read the color map
        int32 ColorMapBytes = (Header.m_ColorMapBits + 7) >> 3;
        ColorMap.resize( Header.m_ColorMapLength * ColorMapBytes );
        for (int i = 0; i < Header.m_ColorMapLength * ColorMapBytes; ++i)
        {
            uint32 ActualBytesRead = pStream->Read( &ColorMap[i], sizeof ColorMap[i] );
            if (ActualBytesRead != sizeof ColorMap[i])
            {
				throw std::runtime_error( "truncated TGA stream" );
            }
        }

    } else {
        switch (Header.m_PixelBits)
        {
            case 8: Desc.Format = EPixelFormat::L8; break;
            case 16: Desc.Format = Header.m_AlphaBits ?
                EPixelFormat::A8R8G8B8 : EPixelFormat::X8R8G8B8;
                break;
            case 24: Desc.Format = EPixelFormat::X8R8G8B8; break;
            case 32: Desc.Format = EPixelFormat::A8R8G8B8; break;
            default:
                assert( false );
        }
    }

    Desc.Width = Header.m_Width;
    Desc.Height = Header.m_Height;
	Desc.Depth = 1;
    Desc.PaletteSize = 0;

    //If failed to initialize image, give up
    pImage->Init( Desc );

    //Read the description with updated variables (e.g. row pitch)
    Desc = pImage->GetDesc();

    //Now read from the file and write pixels into the pImage

    HImageLock hLock = pImage->Lock( EGraphicsResourceLock::WriteOnly );
    uint8 *pData = hLock->GetDataPtr();

    for (int32 i = 0; i < Header.m_Height; ++i)
    {
        int32 DestY = Header.m_bFlipVertical ? (Header.m_Height - 1 - i) : i;

        uint8* pDestRow = pData + DestY * Desc.RowPitch;

		for (int32 j = 0; j < Header.m_Width; ++j)
        {
            uint32 ActualBytesRead;

	        uint8 R,G,B,A;
            int32 DestX = Header.m_bFlipHorizontal ? (Header.m_Width - 1 - j) : j;

            uint32 Index=0;
            if (Header.m_ColorMapLength)
            {
                switch (Header.m_PixelBits)
                {
                default:
                    assert( 0 );
                case 0:
                    break;
                case 8:
                    {
                        uint8 Idx;
                        ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &Idx ), sizeof Idx );
                        if (ActualBytesRead != sizeof Idx)
                        {
                            throw false;
                        }
                        Index = Idx;
                    }
                    break;
                case 16:
                    {
                        uint16 Idx;
                        ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &Idx ), sizeof Idx );
                        if (ActualBytesRead != sizeof Idx)
                        {
                            throw false;
                        }
                        Index = Idx;
                    }
                    break;
                }
            }

            switch (Desc.Format)
            {
            case EPixelFormat::L8:
                ActualBytesRead = pStream->Read(&B, sizeof B);
                if (ActualBytesRead != sizeof B)
                {
                    throw false;
                }
                pDestRow[DestX] = B;
                break;
            case EPixelFormat::X8R8G8B8:
                if (Header.m_ColorMapLength)
                {
                    assert( Header.m_ColorMapBits == 24 );
                    Index *= 3;
                    pDestRow[DestX * 4 + 0] = ColorMap[Index + 0];
                    pDestRow[DestX * 4 + 1] = ColorMap[Index + 1];
                    pDestRow[DestX * 4 + 2] = ColorMap[Index + 2];
                } else if (Header.m_PixelBits == 16) {
                    //2 byte entries are stored as A1R5G5B5
                    uint16 ARGB;
                    ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &ARGB ), sizeof ARGB );
                    if (ActualBytesRead != sizeof ARGB)
                    {
                        throw false;
                    }
                    pDestRow[DestX * 4 + 0] = static_cast<uint8>( (ARGB & 0x001fu) << 3 );
                    pDestRow[DestX * 4 + 1] = static_cast<uint8>( (ARGB & 0x03e0u) >> 2 );
                    pDestRow[DestX * 4 + 2] = static_cast<uint8>( (ARGB & 0x7c00u) >> 7 );
                } else {
                    ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &B ), sizeof B );
                    if (ActualBytesRead != sizeof B)
                    {
                        throw false;
                    }
                    ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &G ), sizeof G );
                    if (ActualBytesRead != sizeof G)
                    {
                        throw false;
                    }
                    ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &R ), sizeof R );
                    if (ActualBytesRead != sizeof R)
                    {
                        throw false;
                    }
                    pDestRow[DestX * 4 + 0] = B;
                    pDestRow[DestX * 4 + 1] = G;
                    pDestRow[DestX * 4 + 2] = R;
                }
                break;
            case EPixelFormat::A8R8G8B8:
                if (Header.m_ColorMapLength)
                {
                    if (Header.m_ColorMapBits == 32)
                    {
                        Index *= 4;
                        pDestRow[DestX * 4 + 0] = ColorMap[Index + 0];
                        pDestRow[DestX * 4 + 1] = ColorMap[Index + 1];
                        pDestRow[DestX * 4 + 2] = ColorMap[Index + 2];
                        pDestRow[DestX * 4 + 3] = ColorMap[Index + 2];
                    } else {
                        assert( Header.m_ColorMapBits == 16 );
                        //2 byte entries are stored as A1R5G5B5
                        uint16 ARGB = reinterpret_cast<uint16*>(&ColorMap[0])[Index];
                        pDestRow[DestX * 4 + 0] = static_cast<uint8>( (ARGB & 0x001fu) << 3 );
                        pDestRow[DestX * 4 + 1] = static_cast<uint8>( (ARGB & 0x03e0u) >> 2 );
                        pDestRow[DestX * 4 + 2] = static_cast<uint8>( (ARGB & 0x7c00u) >> 7 );
                        pDestRow[DestX * 4 + 3] = (ARGB & 0x8000u) ? 0xff : 0x00;
                    }
                } else if (Header.m_PixelBits == 16) {
                    //2 byte entries are stored as A1R5G5B5
                    uint16 ARGB;
                    ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &ARGB ), sizeof ARGB );
                    if (ActualBytesRead != sizeof ARGB)
                    {
                        throw false;
                    }
                    pDestRow[DestX * 4 + 0] = static_cast<uint8>( (ARGB & 0x001fu) << 3 );
                    pDestRow[DestX * 4 + 1] = static_cast<uint8>( (ARGB & 0x03e0u) >> 2 );
                    pDestRow[DestX * 4 + 2] = static_cast<uint8>( (ARGB & 0x7c00u) >> 7 );
                    pDestRow[DestX * 4 + 3] = (ARGB & 0x8000u) ? 0xff : 0x00;
                } else {
                    ActualBytesRead = pStream->Read(reinterpret_cast<uint8*>( &B ), sizeof B );
                    if (ActualBytesRead != sizeof B)
                    {
                        throw false;
                    }
                    ActualBytesRead = pStream->Read(reinterpret_cast<uint8*>( &G ), sizeof G );
                    if (ActualBytesRead != sizeof G)
                    {
                        throw false;
                    }
                    ActualBytesRead = pStream->Read(reinterpret_cast<uint8*>( &R ), sizeof R );
                    if (ActualBytesRead != sizeof R)
                    {
                        throw false;
                    }
                    ActualBytesRead = pStream->Read(reinterpret_cast<uint8*>( &A ), sizeof A );
                    if (ActualBytesRead != sizeof A)
                    {
                        throw false;
                    }
                    pDestRow[DestX * 4 + 0] = B;
                    pDestRow[DestX * 4 + 1] = G;
                    pDestRow[DestX * 4 + 2] = R;
                    pDestRow[DestX * 4 + 3] = A;
                }
                break;
            }
		}
	}
}

/*virtual*/ int32 CImageSerializerTGA::Encode( CStream* pStream, CImage const* pImage ) const
{
    //If image or stream pointers are NULL, then fail
    if (pImage == NULL || pStream == NULL)
    {
        return 0;
    }

	CImage::Desc Desc = pImage->GetDesc();
    
    //If image is locked or has the wrong format, then fail
    if (pImage->IsLocked() || pImage->IsPaletteLocked() ||
		Desc.Depth != 1)
    {
        return 0;
    }

	switch (Desc.Format)
	{
        case EPixelFormat::P8_R8G8B8: 
        case EPixelFormat::L8: 
        case EPixelFormat::X8R8G8B8:
		case EPixelFormat::R32F:
        case EPixelFormat::A8R8G8B8: 
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
			break;
		default:
			return 0; //Unsupported format.
	}
    
    CTGAHeader Header;
    Header.m_Width = (uint16)pImage->GetWidth();
    Header.m_Height = (uint16)pImage->GetHeight();
    Header.m_AlphaBits = 0;
    Header.m_ColorMapLength = 0;
    Header.m_ColorMapBits = 0;
    Header.m_bFlipHorizontal = false;
    Header.m_bFlipVertical = false;

    switch (Desc.Format)
    {
        default:
            assert( 0 );
        case EPixelFormat::P8_R8G8B8: 
            Header.m_PixelBits = 8; 
            Header.m_ColorMapLength = pImage->GetPaletteSize();
            Header.m_ColorMapBits = 24;
            break;
        case EPixelFormat::L8: 
            Header.m_PixelBits = 8; 
            Header.m_AlphaBits = 8;
            break;
        case EPixelFormat::X8R8G8B8:
		case EPixelFormat::R32F:
            Header.m_PixelBits = 24; 
            break;
        case EPixelFormat::A8R8G8B8: 
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
            Header.m_PixelBits = 32; 
            Header.m_AlphaBits = 8;
            break;
    }

    //Lock image data
    HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
    uint8 *pData = hLock->GetDataPtr();

	HImageLock hLockPal = const_cast<CImage*>( pImage )->LockPalette( EGraphicsResourceLock::ReadOnly );
	uint8 *pPal = hLockPal->GetDataPtr();

    uint32 OrigPosition = pStream->GetPosition();

    if (!Header.Encode(pStream))
    {
        return 0;
    }

    if (Header.m_ColorMapLength)
    {
        //Read the palette from pImage and write color map data into the file
        assert( pImage->GetFormat() == EPixelFormat::P8_R8G8B8 );
        
        for (int i = 0; i < Header.m_ColorMapLength; ++i)
        {
            pStream->Write(&pPal[i * 3 + 2], sizeof pPal[i * 3 + 2]);
            pStream->Write(&pPal[i * 3 + 1], sizeof pPal[i * 3 + 1]);
            pStream->Write(&pPal[i * 3 + 0], sizeof pPal[i * 3 + 0]);
        }
    }

    //Now read pixels from the pImage and write data into the file
    for (int32 i = 0; i < Header.m_Height; ++i)
    {
        uint8 const* pSrcRow = pData + i * Desc.RowPitch;

		for (int32 j = 0; j < Header.m_Width; ++j)
        {
	        uint8 R,G,B,A;
            int32 SrcX = Header.m_bFlipHorizontal ? (Header.m_Width - 1 - j) : j;

            //In TGA data,
            //2 byte entries broken down as GGGBBBBB ARRRRRGG
            //3 bytes entries are BGR
            //4 byte entries are BGRA

            switch (Desc.Format)
            {
			default:
				assert( false );
			case EPixelFormat::P8_R8G8B8: 
		    case EPixelFormat::L8: 
                B = pSrcRow[SrcX];
                pStream->Write( reinterpret_cast<uint8*>( &B ), sizeof B );
                break;
		    case EPixelFormat::X8R8G8B8:
                B = pSrcRow[SrcX * 4 + 0];
                G = pSrcRow[SrcX * 4 + 1];
                R = pSrcRow[SrcX * 4 + 2];
                pStream->Write( reinterpret_cast<uint8*>( &B ), sizeof B);
                pStream->Write(&G, sizeof G);
                pStream->Write(&R, sizeof R);
                break;
		    case EPixelFormat::A8R8G8B8: 
                B = pSrcRow[SrcX * 4 + 0];
                G = pSrcRow[SrcX * 4 + 1];
                R = pSrcRow[SrcX * 4 + 2];
                A = pSrcRow[SrcX * 4 + 3];
                pStream->Write(&B, sizeof B);
                pStream->Write(&G, sizeof G);
                pStream->Write(&R, sizeof R);
                pStream->Write(&A, sizeof A);
                break;
			case EPixelFormat::R32F:
				{
					int32 r = Float32ToInt32( reinterpret_cast<float32 const*>(pSrcRow)[SrcX] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					uint8 B = 0xffu;
					pStream->Write( reinterpret_cast<uint8*>( &B ), sizeof B );
					pStream->Write( reinterpret_cast<uint8*>( &B ), sizeof B ); //G
					pStream->Write( reinterpret_cast<uint8*>( &r ), sizeof( uint8 ) ); //R
				}
				break;
			case EPixelFormat::A32B32G32R32F:
				{
					int32 r = Float32ToInt32( reinterpret_cast<float32 const*>(pSrcRow)[4 * SrcX + 0] );
					int32 g = Float32ToInt32( reinterpret_cast<float32 const*>(pSrcRow)[4 * SrcX + 1] );
					int32 b = Float32ToInt32( reinterpret_cast<float32 const*>(pSrcRow)[4 * SrcX + 2] );
					int32 a = Float32ToInt32( reinterpret_cast<float32 const*>(pSrcRow)[4 * SrcX + 3] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					if (a < 0) a = 0; else if (a > 255) a = 255;
					pStream->Write( reinterpret_cast<uint8*>( &b ), sizeof( uint8 ) ); //B
					pStream->Write( reinterpret_cast<uint8*>( &g ), sizeof( uint8 ) ); //G
					pStream->Write( reinterpret_cast<uint8*>( &r ), sizeof( uint8 ) ); //R
					pStream->Write( reinterpret_cast<uint8*>( &a ), sizeof( uint8 ) ); //A
				}
				break;
			case EPixelFormat::A16B16G16R16F:
				{
					int32 r = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>(pSrcRow)[4 * SrcX + 0] ) );
					int32 g = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>(pSrcRow)[4 * SrcX + 1] ) );
					int32 b = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>(pSrcRow)[4 * SrcX + 2] ) );
					int32 a = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>(pSrcRow)[4 * SrcX + 3] ) );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					if (a < 0) a = 0; else if (a > 255) a = 255;
					pStream->Write( reinterpret_cast<uint8*>( &b ), sizeof( uint8 ) ); //B
					pStream->Write( reinterpret_cast<uint8*>( &g ), sizeof( uint8 ) ); //G
					pStream->Write( reinterpret_cast<uint8*>( &r ), sizeof( uint8 ) ); //R
					pStream->Write( reinterpret_cast<uint8*>( &a ), sizeof( uint8 ) ); //A
				}
				break;
            }
		}
	}

	return pStream->GetPosition() - OrigPosition;
}

} //namespace Graphics
} //namespace WT
