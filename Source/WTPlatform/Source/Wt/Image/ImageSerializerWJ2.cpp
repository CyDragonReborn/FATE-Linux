/***************************************************************************************
 *
 * ImageSerializerWJ2.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/9/2004: Gary Hinger: created file
 *
 ***************************************************************************************/

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/stdafx.h"

#include <algorithm>
#include <cassert>
#include "WT/Image/Image.h"
#include "jasper/jasper.h"

using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CImageSerializerWJ2 )

/*virtual*/ bool CImageSerializerWJ2::Validate( CStream* pStream ) const
{
    if (pStream == NULL)
    {
        return false;
    }

    uint8 Header[8];
    int32 ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( Header ), sizeof Header );
	pStream->Seek( -ActualBytesRead, SeekOrigin::Current );
    
    if (ActualBytesRead != sizeof Header)
    {
        return false;
    }

    if ((Header[0] == 0xff && Header[1] == 0x4f) ||
        (Header[4] == 0x6a && Header[5] == 0x50 &&
         Header[6] == 0x20 && Header[7] == 0x20))
    {
        return true;
    }

    return false;
}

CImageSerializerWJ2::CImageSerializerWJ2()
{
    m_CompressionRate = 1.0f;
    std::fill(m_bComponentsLossless, m_bComponentsLossless + 4, false);
    m_LuminanceWeight = 1.0f;
}

void CImageSerializerWJ2::Decode(CStream* pStream, CImage* pImage) const
{
    if (pStream == NULL || pImage == NULL)
    {
		throw std::invalid_argument( "NULL pointer" );
    }

    uint32 StreamLength = pStream->GetLength();

    if (StreamLength == 0)
    {
		throw std::runtime_error( "empty stream" );
    }
    
	std::vector<char> Data;
	Data.resize( StreamLength );

    int32 ActualBytesRead = pStream->Read( reinterpret_cast<uint8*>( &Data[0] ), StreamLength );
    if (static_cast<uint32>( ActualBytesRead ) != StreamLength)
    {
		throw std::runtime_error( "stream length" );
    }

    JasPer::jas_stream_t* in;
    in = JasPer::jas_stream_memopen( &Data[0], StreamLength);

    if (in == NULL)
    {
		throw std::runtime_error( "jas_stream_memopen failure" );
    }

    DecodeJasStream(in, pImage);
}

int32 CImageSerializerWJ2::Encode( CStream* pStream, CImage const* pImage ) const
{
    //If image pointer is NULL, then fail
    if (pImage == NULL || pStream == NULL)
    {
        return 0;
    }
    
    //If image is locked or has the wrong depth, then fail
    if (pImage->IsLocked() || pImage->IsPaletteLocked() ||
		pImage->GetDepth() != 1)
    {
        return 0;
    }

	//Check image format
    switch (pImage->GetFormat())
    {
        case EPixelFormat::P8_R8G8B8:
        case EPixelFormat::A8R8G8B8:
        case EPixelFormat::X8R8G8B8:
        case EPixelFormat::L8:
		case EPixelFormat::R32F:
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
			break;
		default:
			return 0; //Unsupported format.
    }

    JasPer::jas_stream_t* Out;
    Out = JasPer::jas_stream_memopen(NULL, 0);

    if (Out == NULL)
    {
        return 0;
    }
    
    int Result = EncodeJasStream( pImage, Out );

    jas_stream_seek(Out, 0, SEEK_SET);

    //Transfer bytes from the JAS stream to our Stream.
    //Transfer them up to 1024 at a time.
    int BytesLeft = Result;
    while (BytesLeft)
    {
        char Buffer[1024];

        int BytesToRead = sizeof Buffer;
        if (BytesToRead > BytesLeft)
        {
            BytesToRead = BytesLeft;
        }

        jas_stream_read(Out, Buffer, BytesToRead);
        pStream->Write( reinterpret_cast<uint8*>( Buffer ), BytesToRead );
        BytesLeft -= BytesToRead;
    }

    JasPer::jas_stream_close(Out);
    return Result;
}

void CImageSerializerWJ2::DecodeJasStream( void* JasPerStream, CImage* pImage ) const
{
	JasPer::jas_stream_t* in = static_cast<JasPer::jas_stream_t*>( JasPerStream );

	assert( in != NULL );
	assert( pImage != NULL );

    JasPer::jas_image_t* pJasImage = NULL;
    if (!jpc_validate(in))
        pJasImage = jpc_decode(in, "");
    else if (!jp2_validate(in))
        pJasImage = jp2_decode(in, "");

    JasPer::jas_stream_close(in);

    if (pJasImage == NULL)
    {
		throw std::runtime_error( "jp*_decode failure" );
    }

    //Make sure the image is 8-bit precision.
	int NumCmpts = jas_image_numcmpts(pJasImage);
    for (int i = 0; i < NumCmpts; ++i)
    {
        if (jas_image_cmptprec(pJasImage, i) != 8)
        {
            JasPer::jas_image_destroy(pJasImage);
			throw std::runtime_error( "invalid WJ2 component precision" );
        }
    }

    //Check the palette
    if (pJasImage->bUsePalettes)
    {
        if (jas_image_clrspc(pJasImage) != JAS_CLRSPC_SRGB ||
            pJasImage->cmap_numchans != 3 ||
            pJasImage->cmap_ents[0].map != 1 ||
            pJasImage->cmap_ents[1].map != 1 ||
            pJasImage->cmap_ents[2].map != 1 ||
            pJasImage->pclr_bpc[0] != 8 ||
            pJasImage->pclr_bpc[1] != 8 ||
            pJasImage->pclr_bpc[2] != 8 ||
            pJasImage->cmap_ents[0].cmptno != 0 ||
            pJasImage->cmap_ents[1].cmptno != 1 ||
            pJasImage->cmap_ents[2].cmptno != 2)
        {
            JasPer::jas_image_destroy(pJasImage);
			throw std::runtime_error( "WJ2 color space/map" );
        }
    }

    if (pJasImage->bUsePalettes)
    {
        if (NumCmpts != 1 ||
            jas_image_clrspc(pJasImage) != JAS_CLRSPC_SRGB)
        {
            JasPer::jas_image_destroy(pJasImage);
			throw std::runtime_error( "WJ2 component count / color space" );
        }
    } else {
        switch (jas_image_clrspc(pJasImage))
        {
        case JAS_CLRSPC_SRGB:
            if (NumCmpts < 3 ||
                NumCmpts > 4)
            {
                JasPer::jas_image_destroy(pJasImage);
				throw std::runtime_error( "WJ2 component count" );
            }
            break;
        case JAS_CLRSPC_SGRAY:
            if (NumCmpts != 1)
            {
                JasPer::jas_image_destroy(pJasImage);
				throw std::runtime_error( "WJ2 component count" );
            }
            break;
        default:
            JasPer::jas_image_destroy(pJasImage);
			throw std::runtime_error( "WJ2 color space" );
        }
    }

    //The jasper image is OK, now create a CImage.

    CImage::Desc Desc;
    Desc.Width = jas_image_width(pJasImage);
    Desc.Height = jas_image_height(pJasImage);
	Desc.Depth = 1;
    Desc.PaletteSize = pJasImage->bUsePalettes ? pJasImage->pclr_numlutents : 0;
    if (pJasImage->bUsePalettes)
    {
        Desc.Format = EPixelFormat::P8_R8G8B8;
    } else {
        switch (NumCmpts)
        {
        default:
            assert( 0 );
        case 1: Desc.Format = EPixelFormat::L8; break;
        case 3: Desc.Format = EPixelFormat::X8R8G8B8; break;
        case 4: Desc.Format = EPixelFormat::A8R8G8B8; break;
        }
    }

	try
	{
		pImage->Init( Desc );

		//Get new desc with updated pitch fields
		Desc = pImage->GetDesc();

		//Fill in palette data
		if (pImage->GetPaletteSize())
		{
			HImageLock hLockPal = pImage->LockPalette( EGraphicsResourceLock::WriteOnly );
			uint8* pPal = hLockPal->GetDataPtr();
			for (int32 i = 0; i < Desc.PaletteSize * 3; ++i)
				pPal[i] = static_cast<uint8>( pJasImage->pclr_lutdata[i] );
		}

		//Fill in the image data
		//We need to combine the data based on component
		JasPer::jas_matrix_t* pCmptMat[4];
		JasPer::jas_seqent_t* pCmptData[4];
	    
		for (int32 k = 0; k < NumCmpts; ++k)
		{
			pCmptMat[k] = JasPer::jas_matrix_create(1, pImage->GetWidth());
			if (pCmptMat[k] == NULL)
			{
				//Free previously allocated component row data
				for (int32 m = k - 1; m >= 0; --m)
				{
					JasPer::jas_matrix_destroy(pCmptMat[m]);
				}

				throw std::runtime_error( "jas_matrix_create failure" );
			}

			pCmptData[k] = jas_matrix_getref(pCmptMat[k], 0, 0);
		}

		HImageLock hLock = pImage->Lock( EGraphicsResourceLock::WriteOnly );
		uint8* pData = hLock->GetDataPtr();
		for (int32 j = 0; j < pImage->GetHeight(); ++j)
		{
			//Read rows for each component
			for (int32 k = 0; k < NumCmpts; ++k)
			{
				JasPer::jas_image_readcmpt(pJasImage, k, 0, j, pImage->GetWidth(), 1, pCmptMat[k]);
			}

			switch (Desc.Format)
			{
				default:
					assert( 0 );

				case EPixelFormat::P8_R8G8B8:
				case EPixelFormat::L8:
					{
						for (int32 i = 0; i < pImage->GetWidth(); ++i)
						{
							*pData++ = static_cast<uint8>(pCmptData[0][i]);
						}
					}
					break;
				case EPixelFormat::X8R8G8B8:
					{
						for (int32 i = 0; i < pImage->GetWidth(); ++i)
						{
							pData[0] = static_cast<uint8>(pCmptData[2][i]);
							pData[1] = static_cast<uint8>(pCmptData[1][i]);
							pData[2] = static_cast<uint8>(pCmptData[0][i]);
							//Don't set X component
							pData += 4;
						}
					}
					break;
				case EPixelFormat::A8R8G8B8: 
					{
						for (int32 i = 0; i < pImage->GetWidth(); ++i)
						{
							pData[0] = static_cast<uint8>(pCmptData[2][i]);
							pData[1] = static_cast<uint8>(pCmptData[1][i]);
							pData[2] = static_cast<uint8>(pCmptData[0][i]);
							pData[3] = static_cast<uint8>(pCmptData[3][i]);
							pData += 4;
						}
					}
					break;
			}
		}

		for (int32 k = NumCmpts - 1; k >= 0; --k)
		{
			JasPer::jas_matrix_destroy(pCmptMat[k]);
		}
	} catch (...) {
		JasPer::jas_image_destroy(pJasImage);
		throw;
	}
}

//Validation of image locking, format and depth should be done before entering this function
int32 CImageSerializerWJ2::EncodeJasStream( CImage const* pImage, void* JasPerStream ) const
{
	JasPer::jas_stream_t* Out = static_cast<JasPer::jas_stream_t*>( JasPerStream );

    //If image or stream pointers are NULL, then fail
    if (pImage == NULL || Out == NULL)
    {
        return 0;
    }

    //Image should not be locked at this point
    assert( !pImage->IsLocked() );
	assert( !pImage->IsPaletteLocked() );

    int32 Length = 0; //Our return value

	EPixelFormat::Type Format = pImage->GetFormat();
    int32 NumComponents;
    switch (Format)
    {
		default:
			assert( false ); //Format should be valid at this point
        case EPixelFormat::P8_R8G8B8: NumComponents = 1; break;
        case EPixelFormat::A8R8G8B8:  NumComponents = 4; break;
        case EPixelFormat::X8R8G8B8:  NumComponents = 3; break;
        case EPixelFormat::L8:        NumComponents = 1; break;
		case EPixelFormat::R32F:		 NumComponents = 3; break;
		case EPixelFormat::A32B32G32R32F: NumComponents = 4; break;
		case EPixelFormat::A16B16G16R16F: NumComponents = 4; break;
    }

    int32 JasColorSpace = Format == EPixelFormat::L8 ?
        JAS_CLRSPC_SGRAY : JAS_CLRSPC_SRGB;

    JasPer::jas_image_cmptparm_t* cmptparms = (JasPer::jas_image_cmptparm_t*)malloc(NumComponents *
        sizeof(JasPer::jas_image_cmptparm_t));
	for (int cmptno = 0; cmptno < NumComponents; ++cmptno) 
    {
		cmptparms[cmptno].tlx = 0;
		cmptparms[cmptno].tly = 0;
		cmptparms[cmptno].hstep = 1;
		cmptparms[cmptno].vstep = 1;
		cmptparms[cmptno].width = pImage->GetWidth();
		cmptparms[cmptno].height = pImage->GetHeight();
		cmptparms[cmptno].prec = 8;
		cmptparms[cmptno].sgnd = false;
	}

    JasPer::jas_image_t* pJasImage = JasPer::jas_image_create(NumComponents, cmptparms, JasColorSpace);
    free(cmptparms);

    switch (Format)
    {
        default:
            assert( false );
        case EPixelFormat::P8_R8G8B8: //8-bit indexed RGB
            jas_image_setcmpttype(pJasImage, 0, JAS_IMAGE_CT_UNKNOWN);
            break;
        case EPixelFormat::A8R8G8B8: //32-bit RGBA
		case EPixelFormat::A32B32G32R32F:
		case EPixelFormat::A16B16G16R16F:
            jas_image_setcmpttype(pJasImage, 3,JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_OPACITY));
            //Drop down to next case
        case EPixelFormat::X8R8G8B8: //24-bit RGB
		case EPixelFormat::R32F:
            jas_image_setcmpttype(pJasImage, 0,JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_R));
            jas_image_setcmpttype(pJasImage, 1,JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_G));
            jas_image_setcmpttype(pJasImage, 2,JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_RGB_B));
            break;
        case EPixelFormat::L8: //8-bit grayscale
            jas_image_setcmpttype(pJasImage, 0,JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
            break;
    }

    if (Format == EPixelFormat::P8_R8G8B8)
    {
        pJasImage->bUsePalettes = 1;
        pJasImage->cmap_numchans = 3;
        pJasImage->cmap_ents = (JasPer::jas_image_cmapent_t*)JasPer::jas_malloc(
            pJasImage->cmap_numchans * sizeof(JasPer::jas_image_cmapent_t));
        pJasImage->pclr_bpc = (JasPer::uint_fast8_t*)JasPer::jas_malloc(
            3 * sizeof(JasPer::uint_fast8_t));

        for (int i = 0; i < pJasImage->cmap_numchans; ++i)
        {
			pJasImage->cmap_ents[i].cmptno = static_cast<JasPer::uint_fast16_t>( i );
            pJasImage->cmap_ents[i].map = 1;
			pJasImage->cmap_ents[i].pcol = static_cast<JasPer::uint_fast8_t>( i );
            pJasImage->pclr_bpc[i] = 8;
        }
        
        pJasImage->pclr_numlutents = static_cast<JasPer::uint_fast16_t>(pImage->GetPaletteSize());
        pJasImage->pclr_numchans = 3;

        int32 NumBytes = pJasImage->pclr_numchans * pJasImage->pclr_numlutents *
            sizeof(JasPer::int_fast32_t);
        pJasImage->pclr_lutdata = (JasPer::int_fast32_t*)JasPer::jas_malloc(NumBytes);
        memset(pJasImage->pclr_lutdata, 0, NumBytes);
    }

    //Fill in JasPer palette data
    if (pImage->GetPaletteSize())
    {
        HImageLock hLockPal = const_cast<CImage*>( pImage )->LockPalette( EGraphicsResourceLock::ReadOnly );
        uint8* pPal = hLockPal->GetDataPtr();

        for (int32 i = 0; i < pImage->GetPaletteSize() * 3; ++i)
            pJasImage->pclr_lutdata[i] = pPal[i];
    }

    //Fill in the JasPer image data
    //We need to split the data up based on component
    JasPer::jas_matrix_t* pCmptMat[4];
    JasPer::jas_seqent_t* pCmptData[4];

    for (int32 k = 0; k < NumComponents; ++k)
    {
        pCmptMat[k] = JasPer::jas_matrix_create(1, pImage->GetWidth());

        if (pCmptMat[k] == NULL)
        {
            //Free previously allocated component row data
            for (int32 m = k - 1; m >= 0; --m)
            {
                JasPer::jas_matrix_destroy(pCmptMat[m]);
            }

			throw std::runtime_error( "jas_matrix_create failure" );
        }

        pCmptData[k] = jas_matrix_getref(pCmptMat[k], 0, 0);
    }

    HImageLock hLock = const_cast<CImage*>( pImage )->Lock( EGraphicsResourceLock::ReadOnly );
    uint8* pData = hLock->GetDataPtr();

    for (int32 j = 0; j < pImage->GetHeight(); ++j)
    {
        switch (Format)
        {
        default:
            assert( false );
        case EPixelFormat::P8_R8G8B8:
        case EPixelFormat::L8:
            for (int32 i = 0; i < pImage->GetWidth(); ++i)
            {
                pCmptData[0][i] = *pData++;
            }
            break;
        case EPixelFormat::X8R8G8B8:
            for (int32 i = 0; i < pImage->GetWidth(); ++i)
            {
                pCmptData[2][i] = pData[0];
                pCmptData[1][i] = pData[1];
                pCmptData[0][i] = pData[2];
                //Don't use X component
                pData += 4;
            }
            break;
        case EPixelFormat::A8R8G8B8: 
            for (int32 i = 0; i < pImage->GetWidth(); ++i)
            {
                pCmptData[2][i] = pData[0];
                pCmptData[1][i] = pData[1];
                pCmptData[0][i] = pData[2];
                pCmptData[3][i] = pData[3];
                pData += 4;
            }
            break;
		case EPixelFormat::R32F:
            for (int32 i = 0; i < pImage->GetWidth(); ++i)
            {
				int32 r = Float32ToInt32( *reinterpret_cast<float32 const*>( pData ) );
				if (r < 0) r = 0; else if (r > 255) r = 255;
                pCmptData[0][i] = static_cast<uint8>( r ); //R
                pCmptData[1][i] = 0xffu; //G
                pCmptData[2][i] = 0xffu; //B
				pData += sizeof( float32 );
            }
			break;
		case EPixelFormat::A32B32G32R32F:
            for (int32 i = 0; i < pImage->GetWidth(); ++i)
            {
				int32 r = Float32ToInt32( reinterpret_cast<float32 const*>( pData )[0] );
				int32 g = Float32ToInt32( reinterpret_cast<float32 const*>( pData )[1] );
				int32 b = Float32ToInt32( reinterpret_cast<float32 const*>( pData )[2] );
				int32 a = Float32ToInt32( reinterpret_cast<float32 const*>( pData )[3] );
				if (r < 0) r = 0; else if (r > 255) r = 255;
				if (g < 0) g = 0; else if (g > 255) g = 255;
				if (b < 0) b = 0; else if (b > 255) b = 255;
				if (a < 0) a = 0; else if (a > 255) a = 255;
                pCmptData[0][i] = static_cast<uint8>( r ); //R
                pCmptData[1][i] = static_cast<uint8>( g ); //G
                pCmptData[2][i] = static_cast<uint8>( b ); //B
                pCmptData[3][i] = static_cast<uint8>( a ); //A
				pData += 4 * sizeof( float32 );
            }
			break;
		case EPixelFormat::A16B16G16R16F:
            for (int32 i = 0; i < pImage->GetWidth(); ++i)
            {
				int32 r = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pData )[0] ) );
				int32 g = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pData )[1] ) );
				int32 b = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pData )[2] ) );
				int32 a = Float32ToInt32( Float16ToFloat32( reinterpret_cast<float16 const*>( pData )[3] ) );
				if (r < 0) r = 0; else if (r > 255) r = 255;
				if (g < 0) g = 0; else if (g > 255) g = 255;
				if (b < 0) b = 0; else if (b > 255) b = 255;
				if (a < 0) a = 0; else if (a > 255) a = 255;
                pCmptData[0][i] = static_cast<uint8>( r ); //R
                pCmptData[1][i] = static_cast<uint8>( g ); //G
                pCmptData[2][i] = static_cast<uint8>( b ); //B
                pCmptData[3][i] = static_cast<uint8>( a ); //A
				pData += 4 * sizeof( float16 );
            }
			break;
        }

        //Write rows for each component
        for (int32 k = 0; k < NumComponents; ++k)
        {
            JasPer::jas_image_writecmpt(pJasImage, k, 0, j, pImage->GetWidth(), 1, pCmptMat[k]);
        }
    }

    for (int32 k = NumComponents - 1; k >= 0; --k)
    {
        JasPer::jas_matrix_destroy(pCmptMat[k]);
    }

    char OptionsStr[256];
    OptionsStr[0] = '\0';

    //If there is no palette, then pass options
    if (!pImage->GetPaletteSize())
    {
        int32 i;
        for (i = 0; i < NumComponents; ++i)
            if (!m_bComponentsLossless[i])
                break;

        //If the compression rate is less than 1, and not all components are lossless,
        //then pass options
        if (m_CompressionRate < 1.0f && i < NumComponents)
        {
            sprintf(OptionsStr, "mode=real\nrate=%f", m_CompressionRate);
            for (i = 0; i < NumComponents; ++i)
                if (m_bComponentsLossless[i])
                    sprintf(OptionsStr + strlen(OptionsStr), "\nlosslesscomponent=%d", i);

            sprintf(OptionsStr + strlen(OptionsStr), "\nlumaweight=%f", m_LuminanceWeight);
        }
    }
 
    int32 JasResult;
    if (pImage->GetPaletteSize())
    {
        //Use .JP2 format since it contains palette information
        JasResult = jp2_encode( pJasImage, Out, OptionsStr );
    } else {
        //Use more compact .JPC format
        JasResult = jpc_encode( pJasImage, Out, OptionsStr );
    }

    if (!JasResult)
    {
        Length = JasPer::jas_stream_length( Out );
    }

    JasPer::jas_image_destroy( pJasImage );
    return Length;
}

float32 CImageSerializerWJ2::GetCompressionRate()
{
    return m_CompressionRate;
}

bool CImageSerializerWJ2::IsComponentLossless(int32 ComponentIndex)
{
    if (ComponentIndex < 0 || 
        ComponentIndex >= sizeof m_bComponentsLossless / sizeof m_bComponentsLossless[0])
        return false;
    return m_bComponentsLossless[ComponentIndex];
}

float32 CImageSerializerWJ2::GetLuminanceWeight()
{
    return m_LuminanceWeight;
}

bool CImageSerializerWJ2::SetCompressionRate(float32 Rate)
{
    if (Rate < 0.0f || Rate > 1.0f)
    {
        return false;
    }
    m_CompressionRate = Rate;
    return true;
}

bool CImageSerializerWJ2::SetComponentLossless(int32 ComponentIndex, bool bLossless)
{
    if (ComponentIndex < 0 || 
        ComponentIndex >= sizeof m_bComponentsLossless / sizeof m_bComponentsLossless[0])
        return false;
    m_bComponentsLossless[ComponentIndex] = bLossless;
    return true;
}

bool CImageSerializerWJ2::SetLuminanceWeight(float32 LuminanceWeight)
{
    if (LuminanceWeight < 0.0f)
    {
        return false;
    }
    m_LuminanceWeight = LuminanceWeight;
    return true;
}

} //namespace Graphics
} //namespace WT
