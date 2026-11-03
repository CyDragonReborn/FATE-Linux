/***************************************************************************************
 *
 * Texture.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          09/13/2005
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"

#include "WT/Graphics/Texture.h"
#include "WT/System/SystemException.h"
#include "WT/System/Logging/Log.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/IO/FilePath.h"

using namespace WT::System;
using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

float64 gSimulationTimeSeconds = 0.0;

CTexture::TextureCache	CTexture::gm_TextureCache;
CTexture::CombinedCache CTexture::gm_CombinedCache;
CTexture::MovieCache	CTexture::gm_MovieCache;

/*static*/ void WTCall CTexture::FlushTextureCache()
{
	gm_TextureCache.clear();
	gm_CombinedCache.clear();

	LOG_GREEN( "Info: releasing %d cached MovieTexture(s)", static_cast<int32>( gm_MovieCache.size() ) );

	gm_MovieCache.clear();
}

void CTexture::InvalidateDeviceObjects()
{
}

void CTexture::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
}

bool CTexture::IsValid() const
{
	return true;
}

CMovieTexture::CMovieTexture( std::string const& Path, LPDIRECT3DDEVICE9 pDevice, bool bLooping )
		: m_AVITexture( Path, pDevice, bLooping )
		, m_SimulationTimeSeconds( gSimulationTimeSeconds )
{
}

void CMovieTexture::InvalidateDeviceObjects()
{
	m_AVITexture.InvalidateDeviceObjects();
}

void CMovieTexture::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	m_AVITexture.RestoreDeviceObjects( pDevice );
}

bool CMovieTexture::IsValid() const
{
	return m_AVITexture.IsValid();
}

LPDIRECT3DBASETEXTURE9 CMovieTexture::GetD3DTexture()
{
	if (m_SimulationTimeSeconds != gSimulationTimeSeconds)
	{
		m_AVITexture.Update( static_cast<float32>( gSimulationTimeSeconds - m_SimulationTimeSeconds ) );
		m_SimulationTimeSeconds = gSimulationTimeSeconds;
	}

	return m_AVITexture.GetTexture();
}

HImage MyCreateImage( std::string const& FilePath )
{
	HImage hImage;
	try
	{
		hImage = CImage::CreateImage( FilePath );
	} catch (CIOException const&) {
		LOG_PURPLE( "Warning: CIOException reading image file '%s'.", FilePath.c_str() );
	}

	if (hImage == NULL)
	{
		return hImage;
	}

	//Are images too large? Downsample them to increase frame rate on slow GPU's.
	//This feature could be useful for slow PC's.
	//TODO reexamine how this should be used.
#if 0
	while (hImage->GetWidth()  >= 512 &&
		   hImage->GetHeight() >= 512)
	{
#ifndef NDEBUG
		{
			ostringstream o;
			o << "Info: downsampling " << hImage->GetWidth() << "x" << hImage->GetHeight() << " image '" << FilePath << "'\n";
			OutputDebugString( o.str().c_str() );
		}
#endif
		
		//Chop image size in half

		CImage::Desc SrcDesc = hImage->GetDesc();
		CImage::Desc DestDesc = SrcDesc;
		DestDesc.Width >>= 1;
		DestDesc.Height >>= 1;
		HImage hDestImage( CImage::CreateImage( DestDesc ) );
	
		for (int32 z = 0; z < DestDesc.Depth; ++z)
		{
			for (int32 y = 0; y < DestDesc.Height; ++y)
			{
				for (int32 x = 0; x < DestDesc.Width; ++x)
				{
					uint32 ag, rb;

					ag = rb = 0x20002; //rounding term needed for division by 4 later

					//Add 4 pixels from source image
					for (int32 j = 0; j < 2; ++j)
					{
						for (int32 i = 0; i < 2; ++i)
						{
							uint32 const c = hImage->GetPixelColor( (x << 1) + i, (y << 1) + j, z );
							ag += (c >> 8) & 0x00ff00ff;
							rb += c & 0x00ff00ff;
						}
					}

					uint32 const c = ((ag << 6) & 0xff00ff00) + ((rb >> 2) & 0x00ff00ff);

					hDestImage->SetPixelColor( x, y, z, c );
				}
			}
		}

		hImage = hDestImage;
	}
#endif
	
	return hImage;
}

/*static*/ HTexture WTCall CTexture::Load( string const& Path, LPDIRECT3DDEVICE9 pDevice, bool bOpacity, HImage* phImage )
{
	LPDIRECT3D9 pD3D;
	HRESULT Result = pDevice->GetDirect3D( &pD3D );
	if (FAILED( Result ))
	{
		WT_THROW( CRuntimeException, "GetDirect3D failure" );
	}

	if (!Path.empty())
	{
		if ( !stricmp( CFilePath::GetExtension( Path ).c_str() , ".avi" ))
		{
			HTexture hMovie;
			CTexture::MovieCache::iterator it = CTexture::gm_MovieCache.find( Path );
			if (it != CTexture::gm_MovieCache.end())
			{
				hMovie = it->second;
			} else {
				hMovie = HTexture( new CMovieTexture( Path, pDevice, true ) );
				CTexture::gm_MovieCache[Path] = hMovie;
			}

			return hMovie.get();
		} else {
			HImage hImage = MyCreateImage( Path );

			if (phImage != NULL)
			{
				*phImage = hImage;
			}

			CTexture::TextureCache::iterator TextureIt = CTexture::gm_TextureCache.find( Path );
			if (TextureIt != CTexture::gm_TextureCache.end())
			{
				return TextureIt->second.get();
			} else {
				if (hImage != NULL)
				{
					D3DFORMAT Format = D3DFMT_UNKNOWN;

					switch (hImage->GetFormat())
					{
						case EPixelFormat::DXT1: Format = D3DFMT_DXT1; break;
						case EPixelFormat::DXT3: Format = D3DFMT_DXT3; break;
						case EPixelFormat::DXT5: Format = D3DFMT_DXT5; break;
						default:
							{
								static D3DFORMAT const g_Formats[] = 
								{
									D3DFMT_A8R8G8B8,
									D3DFMT_A8B8G8R8,
									D3DFMT_R8G8B8,
									D3DFMT_X8R8G8B8,
									D3DFMT_X8B8G8R8,
									D3DFMT_R5G6B5,
									D3DFMT_UNKNOWN,
								};

								static D3DFORMAT const g_OpacityFormats[] = 
								{
									D3DFMT_A8,
									D3DFMT_A8L8,
									D3DFMT_A8R3G3B2,
									D3DFMT_A8R8G8B8,
									D3DFMT_A8B8G8R8,
									D3DFMT_A4L4,
									D3DFMT_A4R4G4B4,
									D3DFMT_UNKNOWN,
								};

								D3DFORMAT const* pFormats = bOpacity ? g_OpacityFormats : g_Formats;

								D3DDISPLAYMODE Mode;
								HRESULT Result = pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &Mode );
								if (FAILED( Result ))
								{
									throw std::runtime_error( "GetAdapterDisplayMode failure" );
								}
								
								for (int32 i = 0; pFormats[i] != D3DFMT_UNKNOWN; ++i)
								{
									HRESULT Result = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Mode.Format, 
										0, D3DRTYPE_TEXTURE, pFormats[i] );

									if (SUCCEEDED( Result ))
									{
										Format = pFormats[i];
										break;
									}
								}

								if (Format == D3DFMT_UNKNOWN)
								{
									throw std::runtime_error( "no suitable D3DFORMAT found" );
								}
							}
					}

					LPDIRECT3DBASETEXTURE9 pTexture = hImage->CreateD3DTexture( pDevice, D3DUSAGE_AUTOGENMIPMAP,
						Format, D3DPOOL_MANAGED, bOpacity ? CImage::KCreateD3DTextureFlagsAsOpacityMap : 0 );

					HSimpleTexture hSimpleTexture = new CSimpleTexture();
					hSimpleTexture->SetD3DTexture( pTexture );
					if (pTexture != NULL)
					{
						pTexture->Release();
					}

					CTexture::gm_TextureCache[Path] = hSimpleTexture;
					return hSimpleTexture.get();
				}
			}
		}
	}

	return NULL;
}

}
}
