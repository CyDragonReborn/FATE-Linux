
#include "d3dx8_shim.h"
/*
	texture.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	4/29/2004

	Change History

	Travis Baldree
	5/17/2004	-	Textures were being created with alpha-channel by default -
					this was unnecessary and made a mess. So, converttoformat method
					was added so that they could be changed to alpha supporting when
					alpha channels were being merged in. Much more efficient this way,
					and now all textures don't report that they have alpha. (whew)

*/

#ifndef _TEXTURE_H
#define _TEXTURE_H

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/constants.h"

// comment this out if you do not want automatic compressed texture conversion
#define CONVERT_TO_DDS

// enable this if you would like JPEG2000 support - you must have the JASPER library installed
// to use this
//#define _USE_JPEG2000
using namespace std;
// the texture class handles loading of textures, and application of alpha masks and colorkeys
class CTexture
{
public:
						CTexture( bool AllowTextureCompression = kTrue );		// allow DXT texture compression?
						~CTexture( void );

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								std::string TextureName,			// absolute path to texture
								bool ColorKey = kFalse,				// use colorkey?
								DWORD ColorKeyValue = D3DCOLOR_ARGB( 255, 255, 0, 255 ) );	// defaults to magenta, if present

	HRESULT				CreateBlankTexture( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
											uint32 Width,					// width in pixels
											uint32 Height );				// height in pixel

	HRESULT				ConvertToFormat( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
										 D3DFORMAT D3DFormat );				// desired format to convert to	

	HRESULT				ConvertToFormatSystem( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
											   D3DFORMAT D3DFormat );				// desired format to convert to	

	HRESULT				MergeAlpha( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device		
									CTexture&	pAlphaTexture,			// alpha texture to merge
									int32 MipLevels = D3DX_DEFAULT );	// # of mip levels to generate (D3DX_DEFAULT is default)

	void				Update( float32 TimeElapsed );		// seconds elapsed

#ifdef _USE_JPEG2000

	HRESULT				CreateJP2( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								   std::string TextureName,				// absolute path to WJ2
								   LPDIRECT3DTEXTURE8* TargetTexture,	// D3D texture to fill
								   bool ColorKey,						// use colorkey?
								   DWORD ColorKeyValue );					// colorkey value
#endif
	
	bool				IsValid( void )				{	return m_Valid;				};
	
	bool				HasAlpha( void )			{	return m_HasAlpha;			};
	
	bool				HasColorKey( void )			{	return m_HasColorKey;		};
	
	std::string			GetPath( void )				{	return m_Path;				};

	LPDIRECT3DTEXTURE8	GetTexture( void );

	uint32				Width( void )				{	return m_Width;				};
	uint32				Height( void )				{	return m_Height;			};

private:

	uint32				m_Width;
	uint32				m_Height;

	std::string			m_Path;	

	bool				m_Valid;
	bool				m_HasAlpha;
	bool				m_HasColorKey;


	LPDIRECT3DTEXTURE8	m_pTexture;

	bool				m_AllowTextureCompression;

};

#endif