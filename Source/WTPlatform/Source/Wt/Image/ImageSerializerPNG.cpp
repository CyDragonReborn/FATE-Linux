/***************************************************************************************
 *
 * ImageSerializerPNG.h
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

#include <algorithm>
#include <cassert>
#include "WT/Image/Image.h"
#include "png.h"

using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CImageSerializerPNG )

/*virtual*/ bool CImageSerializerPNG::Validate( CStream* pStream ) const
{
    if (pStream == NULL)
    {
        return false;
    }

    int32 const kHeaderSize = 8; //Check 8 bytes using libpng
    uint8 Header[kHeaderSize];

    int32 ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &Header ), sizeof Header );
	pStream->Seek( -ActualBytesRead, SeekOrigin::Current );
    if (ActualBytesRead != sizeof Header)
    {
        //Incomplete header
        return false;
    }

    return libpng::png_sig_cmp( Header, 0, sizeof Header ) ? false : true;
}

//Functions supporting libpng
namespace
{

void PNGAPI my_png_error(libpng::png_structp, libpng::png_const_charp)
{
    throw false;
}

void PNGAPI my_png_warning(libpng::png_structp, libpng::png_const_charp)
{
    //Don't display a warning message
}

void PNGAPI ReadStream(libpng::png_structp png_ptr, libpng::png_bytep data, libpng::png_size_t length)
{
    CStream* pStream = static_cast<CStream*>(libpng::png_get_io_ptr(png_ptr));
    int32 ActualSizeRetrieved = pStream->Read( reinterpret_cast<uint8*>(data), static_cast<int32>( length ) );
    if (static_cast<uint32>( ActualSizeRetrieved ) != length)
    {
        //Error reading data
        libpng::png_error(png_ptr, "");
    }
}

void PNGAPI WriteStream(libpng::png_structp png_ptr, libpng::png_bytep data, libpng::png_size_t length)
{
    CStream* pStream = static_cast<CStream*>(libpng::png_get_io_ptr(png_ptr));
    pStream->Write( reinterpret_cast<uint8*>(data), static_cast<int32>( length ) );
}

void PNGAPI FlushStream(libpng::png_structp /*png_ptr*/)
{
}

} //anonymous namespace

void CImageSerializerPNG::Decode( CStream* pStream, CImage* pImage ) const
{
    if (pStream == NULL)
    {
		throw std::invalid_argument( "NULL stream" );
    }

    if (pImage == NULL)
    {
		throw std::invalid_argument( "NULL image" );
    }

    libpng::png_structp png_ptr = libpng::png_create_read_struct(PNG_LIBPNG_VER_STRING, 
        NULL, my_png_error, my_png_warning);
    if (png_ptr == NULL)
    {
		throw std::runtime_error( "png_create_read_struct failure" );
    }

    libpng::png_infop info_ptr = libpng::png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        libpng::png_destroy_read_struct(&png_ptr, (libpng::png_infopp)NULL, (libpng::png_infopp)NULL);
		throw std::runtime_error( "png_create_info_struct failure" );
    }

	std::vector<libpng::png_bytep> Rows;
    libpng::png_uint_32 height;

    try
    {
        libpng::png_set_read_fn(png_ptr, pStream, ReadStream);

        libpng::png_set_sig_bytes(png_ptr, 0);
        libpng::png_read_info(png_ptr, info_ptr);

        libpng::png_uint_32 width;
        int bit_depth, color_type, interlace_type;
        libpng::png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
            &interlace_type, int_p_NULL, int_p_NULL);

        //tell libpng to strip 16 bit/color files down to 8 bits/color
	    libpng::png_set_strip_16(png_ptr);

        //Extract multiple pixels with bit depths of 1, 2, and 4 from a single
        //byte into separate bytes (useful for paletted and grayscale images).
	    libpng::png_set_packing(png_ptr);

        libpng::png_set_expand(png_ptr);

		libpng::png_read_update_info(png_ptr, info_ptr);

        CImage::Desc Desc;
		switch (info_ptr->color_type)
		{
			case PNG_COLOR_TYPE_GRAY:
			case PNG_COLOR_TYPE_GRAY_ALPHA:
                Desc.Format = EPixelFormat::L8;
				break;

			case PNG_COLOR_TYPE_RGB_ALPHA:
                Desc.Format = EPixelFormat::A8R8G8B8;
				break;

			default:
                Desc.Format = EPixelFormat::X8R8G8B8;
				break;
		}
        Desc.Width = width;
        Desc.Height = height;
		Desc.Depth = 1;
        Desc.PaletteSize = 0;
        pImage->Init(Desc);

        //Get the image description again, including updated pitch information
        Desc = pImage->GetDesc();

	    //Lock image data
		HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
		uint8 *pData = hLock->GetDataPtr();
    	//HImageLock LockPal = const_cast<CImage*>( pImage )->LockPalette( EGraphicsResourceLock::ReadOnly );
		//uint8 *pPal = hLockPal->GetDataPtr();

        int32 RowPitch = libpng::png_get_rowbytes (png_ptr, info_ptr);

        Rows.resize( height );
        for (uint32 i = 0; i < height; ++i)
        {
			//TODO fix potential leak here if exception is thrown before delete[] called
            Rows[i] = new libpng::png_byte[RowPitch];
        }

        libpng::png_read_image(png_ptr, &Rows[0] );

		//Read rest of file, and get additional chunks in info_ptr - REQUIRED
        libpng::png_read_end(png_ptr, info_ptr);

        for (int32 i = 0; i < Desc.Height; ++i)
        {
            uint8* pDestRow = pData + i * Desc.RowPitch;

            switch (Desc.Format)
            {
            default:
                assert( false );
                throw false;
            case EPixelFormat::L8:
                std::copy( Rows[i], Rows[i] + Desc.Width, pDestRow);
                break;
            case EPixelFormat::X8R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    pDestRow[j * 4 + 0] = Rows[i][j * 3 + 2]; //B
                    pDestRow[j * 4 + 1] = Rows[i][j * 3 + 1]; //G
                    pDestRow[j * 4 + 2] = Rows[i][j * 3 + 0]; //R
                }
                break;
            case EPixelFormat::A8R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    pDestRow[j * 4 + 0] = Rows[i][j * 4 + 2]; //B
                    pDestRow[j * 4 + 1] = Rows[i][j * 4 + 1]; //G
                    pDestRow[j * 4 + 2] = Rows[i][j * 4 + 0]; //R
                    pDestRow[j * 4 + 3] = Rows[i][j * 4 + 3]; //A
                }
                break;
            }
        }

        for (uint32 i = 0; i < height; ++i)
        {
            delete[] Rows[i];
        }
		Rows.clear();

    } catch (...) {
		libpng::png_destroy_read_struct(&png_ptr, &info_ptr, (libpng::png_infopp)NULL);
        throw;
    }
  
    libpng::png_destroy_read_struct(&png_ptr, &info_ptr, (libpng::png_infopp)NULL);
}

int32 CImageSerializerPNG::Encode( CStream* pStream, CImage const* pImage ) const
{
    //If image pointer is NULL, then fail
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
		case EPixelFormat::L8:
        case EPixelFormat::P8_R8G8B8:
        case EPixelFormat::X8R8G8B8:
		case EPixelFormat::R32F:
        case EPixelFormat::A8R8G8B8:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
			break;
		default:
			return 0; //Unsupported format.
	}

    uint32 OrigPosition = pStream->GetPosition();
        
    libpng::png_structp png_ptr;
    libpng::png_infop info_ptr;
	std::vector<libpng::png_color> palette;

	std::vector<libpng::png_byte> Row;
    
    //Create and initialize the png_struct with the desired error handler functions.
    png_ptr = libpng::png_create_write_struct(PNG_LIBPNG_VER_STRING,
        NULL, my_png_error, my_png_warning);
    if (png_ptr == NULL)
    {
        return 0;
    }

    //Allocate/initialize the image information data. REQUIRED
    info_ptr = libpng::png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        libpng::png_destroy_write_struct(&png_ptr, NULL);
        return 0;
    }
    
    try
    {
        //Initialize I/O
        libpng::png_set_write_fn(png_ptr, pStream, WriteStream, FlushStream);

        //Set the zlib compression level
        libpng::png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

        int color_type;
        uint32 bytes_per_pixel;
        switch (Desc.Format)
        {
        default:
            assert( false );
		case EPixelFormat::L8:
            color_type = PNG_COLOR_TYPE_GRAY;
            bytes_per_pixel = 1;
            break;
        case EPixelFormat::P8_R8G8B8:
            color_type = PNG_COLOR_TYPE_PALETTE;
            bytes_per_pixel = 1;
            break;
        case EPixelFormat::X8R8G8B8:
		case EPixelFormat::R32F:
            color_type = PNG_COLOR_TYPE_RGB;
            bytes_per_pixel = 3;
            break;
        case EPixelFormat::A8R8G8B8:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
            color_type = PNG_COLOR_TYPE_RGB_ALPHA;
            bytes_per_pixel = 4;
            break;
        }

        //Set the image information.
        libpng::png_set_IHDR(png_ptr, info_ptr, Desc.Width, Desc.Height, 8, color_type,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);


	    HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
        uint8* pData = hLock->GetDataPtr();

        if (Desc.PaletteSize)
        {
            HImageLock hLockPal = const_cast<CImage*>( pImage )->LockPalette( EGraphicsResourceLock::ReadOnly );
            uint8 *pPal = hLockPal->GetDataPtr();

            //Set the palette if there is one.  REQUIRED for indexed-color images
            palette.resize( PNG_MAX_PALETTE_LENGTH );

            //Set palette colors
            int32 i;
            for (i = 0; i < Desc.PaletteSize; ++i)
            {
                palette[i].red = pPal[i * 3 + 0];
                palette[i].green = pPal[i * 3 + 1];
                palette[i].blue = pPal[i * 3 + 2];
            }

            for (; i < PNG_MAX_PALETTE_LENGTH; ++i)
            {
                palette[i].red = 0;
                palette[i].green = 0;
                palette[i].blue = 0;
            }

            //You must not free palette here, because png_set_PLTE only makes a link to
            //the palette that you malloced.  Wait until you are about to destroy
            //the png structure.
            png_set_PLTE(png_ptr, info_ptr, &palette[0], PNG_MAX_PALETTE_LENGTH);
        }
        
        //Write the file header information.  REQUIRED
        png_write_info(png_ptr, info_ptr);
        
        Row.resize( Desc.Width * bytes_per_pixel );

        for (int32 i = 0; i < Desc.Height; ++i)
        {
            uint8 const* pSrcRow = pData + i * Desc.RowPitch;

            switch (Desc.Format)
            {
			default:
				assert( false );
            case EPixelFormat::L8:
                std::copy(pSrcRow, pSrcRow + Desc.Width, &Row[0] );
                break;
            case EPixelFormat::X8R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    Row[j * 3 + 0] = pSrcRow[j * 4 + 2]; //R
                    Row[j * 3 + 1] = pSrcRow[j * 4 + 1]; //G
                    Row[j * 3 + 2] = pSrcRow[j * 4 + 0]; //B
                }
                break;
            case EPixelFormat::A8R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    Row[j * 4 + 0] = pSrcRow[j * 4 + 2]; //R
                    Row[j * 4 + 1] = pSrcRow[j * 4 + 1]; //G
                    Row[j * 4 + 2] = pSrcRow[j * 4 + 0]; //B
                    Row[j * 4 + 3] = pSrcRow[j * 4 + 3]; //A
                }
                break;
            case EPixelFormat::P8_R8G8B8:
                for (int32 j = 0; j < Desc.Width; ++j)
                {
                    Row[j] = pSrcRow[j];
                }
                break;
			case EPixelFormat::R32F:
				for (int32 j = 0; j < Desc.Width; ++j)
				{
 					int32 r = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[j] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					Row[j * 3 + 0] = static_cast<uint8>( r ); //R
					Row[j * 3 + 1] = 0xff; //G
					Row[j * 3 + 2] = 0xff; //B
				}
				break;
			case EPixelFormat::A32B32G32R32F:
				for (int32 j = 0; j < Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 0] );
					int32 g = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 1] );
					int32 b = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 2] );
					int32 a = Float32ToInt32( reinterpret_cast<float32 const*>( pSrcRow )[4 * j + 3] );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					if (a < 0) a = 0; else if (a > 255) a = 255;
					Row[j * 4 + 0] = static_cast<uint8>( r ); //R
					Row[j * 4 + 1] = static_cast<uint8>( g ); //G
					Row[j * 4 + 2] = static_cast<uint8>( b ); //B
					Row[j * 4 + 3] = static_cast<uint8>( a ); //A
				}
				break;
			case EPixelFormat::A16B16G16R16F:
				for (int32 j = 0; j < Desc.Width; ++j)
				{
					int32 r = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 0] ) );
					int32 g = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 1] ) );
					int32 b = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 2] ) );
					int32 a = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pSrcRow )[4 * j + 3] ) );
					if (r < 0) r = 0; else if (r > 255) r = 255;
					if (g < 0) g = 0; else if (g > 255) g = 255;
					if (b < 0) b = 0; else if (b > 255) b = 255;
					if (a < 0) a = 0; else if (a > 255) a = 255;
					Row[j * 4 + 0] = static_cast<uint8>( r ); //R
					Row[j * 4 + 1] = static_cast<uint8>( g ); //G
					Row[j * 4 + 2] = static_cast<uint8>( b ); //B
					Row[j * 4 + 3] = static_cast<uint8>( a ); //A
				}
				break;
			}

			WT::Graphics::libpng::png_bytep pRow = &Row[0];
            png_write_rows(png_ptr, &pRow, 1);
        }

		Row.clear();

        //It is REQUIRED to call this to finish writing the rest of the file
        libpng::png_write_end(png_ptr, info_ptr);
       
    } catch (...) {
        //If we get here, we had a problem reading the file
        libpng::png_destroy_write_struct(&png_ptr, &info_ptr);
		throw;
    }

    //Clean up after the write, and free any memory allocated
    libpng::png_destroy_write_struct(&png_ptr, &info_ptr);

    return pStream->GetPosition() - OrigPosition;
}

} //namespace Graphics
} //namespace WT
