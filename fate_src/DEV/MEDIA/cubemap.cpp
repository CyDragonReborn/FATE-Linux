#include "d3dx8.h"
/*
	cubemap.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	4/21/2004

*/


#include <D3D8.h>
#include "D3DX8.h"
#include <assert.h>


#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "bitmap.h"

#include "material.h"
#include "texture.h"
#include "cubemap.h"


CCubeMap::CCubeMap( void ) :
		m_Valid( kFalse ),
		m_pTexture( NULL ),
		m_pRenderToCubeMap( NULL ),
		m_Type( KCubeStandard ),
		m_Width( 64 ),
		m_Path( "" ),
		m_RightPath( "" ),
		m_LeftPath( "" ),
		m_UpPath( "" ),
		m_DownPath( "" ),
		m_FrontPath( "" ),
		m_BackPath( "" )
{
} // CCubeMap::CCubeMap()


CCubeMap::~CCubeMap()
{
	RELEASE_SAFELY( m_pTexture );
	RELEASE_SAFELY( m_pRenderToCubeMap );
} // CCubeMap::~CCubeMap()

void CCubeMap::InvalidateDeviceObjects( void )
{
	if( m_Valid )
	{
		RELEASE_SAFELY( m_pTexture );
		RELEASE_SAFELY( m_pRenderToCubeMap );
		m_Valid = kFalse;
	}
} // CCubeMap::InvalidateDeviceObjects()

void CCubeMap::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if( m_Valid )
	{
		return;
	}

	switch ( m_Type )
	{
	case KCubeFromFile :
		Create( pD3DDevice, m_Path );
		break;
	case KCubeFromFiles :
		Create( pD3DDevice, 
				m_RightPath,
				m_LeftPath,
				m_UpPath,
				m_DownPath,
				m_FrontPath,
				m_BackPath );
		break;
	case KCubeStandard :
	case KCubeRenderTarget :
		Create( pD3DDevice, m_Width, m_Type );
		break;
	}

} // CCubeMap::RestoreDeviceObjects()

// create cubemap from 6 files
HRESULT CCubeMap::Create( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
						  const std::string& Right,		// path to copy from
						  const std::string& Left,		// path to copy from
						  const std::string& Up,		// path to copy from
						  const std::string& Down,		// path to copy from
						  const std::string& Front,		// path to copy from
						  const std::string& Back )		// path to copy from
{
	m_RightPath = Right;
	m_LeftPath = Left;
	m_UpPath = Up;
	m_DownPath = Down;
	m_FrontPath = Front;
	m_BackPath = Back;

	// create a material from each path
	CTexture* pRight = new CTexture();
	pRight->Create( pD3DDevice, Right );

	CTexture* pLeft= new CTexture();
	pLeft->Create( pD3DDevice, Left );

	CTexture* pUp = new CTexture();
	pUp->Create( pD3DDevice, Up );

	CTexture* pDown = new CTexture();
	pDown->Create( pD3DDevice, Down );

	CTexture* pFront = new CTexture();
	pFront->Create( pD3DDevice, Front );

	CTexture* pBack = new CTexture();
	pBack->Create( pD3DDevice, Back );

	// use the textures of the materials to create a cubemap
	HRESULT hr =  Create( pD3DDevice,
						  pRight->GetTexture(),
						  pLeft->GetTexture(),
						  pUp->GetTexture(),
						  pDown->GetTexture(),
						  pFront->GetTexture(),
						  pBack->GetTexture() );

	DELETE_SAFELY( pRight );
	DELETE_SAFELY( pLeft );
	DELETE_SAFELY( pUp );
	DELETE_SAFELY( pDown );
	DELETE_SAFELY( pFront );
	DELETE_SAFELY( pBack );

	m_Type = KCubeFromFiles;

	return hr;
} // CCubeMap::Create()

// create a cubemap from 6 materials
HRESULT CCubeMap::Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						  CMaterial& pRight,		// texture to copy from
						  CMaterial& pLeft,			// texture to copy from
						  CMaterial& pUp,			// texture to copy from
						  CMaterial& pDown,			// texture to copy from
						  CMaterial& pFront,		// texture to copy from
						  CMaterial& pBack )		// texture to copy from						  
{
	return Create( pD3DDevice,
				   pRight.GetTexture(),
				   pLeft.GetTexture(),
				   pUp.GetTexture(),
				   pDown.GetTexture(),
				   pFront.GetTexture(),
				   pBack.GetTexture() );
} // CCubeMap::Create()

// create a cubemap from 6 textures
HRESULT CCubeMap::Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						  LPDIRECT3DTEXTURE8 pRight,		// texture to copy from
						  LPDIRECT3DTEXTURE8 pLeft,			// texture to copy from
						  LPDIRECT3DTEXTURE8 pUp,			// texture to copy from
						  LPDIRECT3DTEXTURE8 pDown,			// texture to copy from
						  LPDIRECT3DTEXTURE8 pFront,		// texture to copy from
						  LPDIRECT3DTEXTURE8 pBack )		// texture to copy from						  
{
	// get a pointer to the backbuffer so we can check its format -
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	pRight->GetSurfaceLevel( 0, &pSurfaceSrc );

	D3DSURFACE_DESC	Desc;
	pSurfaceSrc->GetDesc( &Desc );
	
	RELEASE_SAFELY( pSurfaceSrc );


	HRESULT hr = Create( pD3DDevice, Desc.Width, KCubeStandard );

	// if creation of the texture failed, return
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}
	CopyToFace( pD3DDevice, D3DCUBEMAP_FACE_POSITIVE_X, pRight );
	CopyToFace( pD3DDevice, D3DCUBEMAP_FACE_NEGATIVE_X, pLeft );
	CopyToFace( pD3DDevice, D3DCUBEMAP_FACE_POSITIVE_Y, pUp );
	CopyToFace( pD3DDevice, D3DCUBEMAP_FACE_NEGATIVE_Y, pDown );
	CopyToFace( pD3DDevice, D3DCUBEMAP_FACE_POSITIVE_Z, pFront );
	CopyToFace( pD3DDevice, D3DCUBEMAP_FACE_NEGATIVE_Z, pBack );

	return S_OK;
} // CCubeMap::Create()

// begins rendering to cube map
HRESULT CCubeMap::BeginRender( void )
{
	if( m_Type != KCubeRenderTarget )
	{
		return E_FAIL;
	}

	assert( m_pRenderToCubeMap != NULL );


	HRESULT hr = m_pRenderToCubeMap->BeginCube( m_pTexture );

	return hr;
} // CCubeMap::BeginRender()

// sets face to currently render on for the cubemap
HRESULT CCubeMap::SetRenderFace( D3DCUBEMAP_FACES Face )	// face to begin a render on
{
	if( m_Type != KCubeRenderTarget )
	{
		return E_FAIL;
	}

	assert( m_pRenderToCubeMap != NULL );


	HRESULT hr = m_pRenderToCubeMap->Face( Face );

	return hr;
} // CCubeMap::SetRenderFace()

// end an active render to a face
void CCubeMap::EndRender( void )	
{
	assert( m_pRenderToCubeMap != NULL );

	m_pRenderToCubeMap->End();
} // CCubeMap::EndRender()

// create an empty cubemap
HRESULT CCubeMap::Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						  uint32 Width,						// width of cube
						  ECubeMapType Type )				// render target or standard
{
	m_Valid = kFalse;

	// clean up any previous texture
	InvalidateDeviceObjects();

	m_Type = Type;
	m_Width = Width;

	if( m_Type == KCubeFromFile )
	{
		m_Type = KCubeStandard;
	}

	int32 Format( 0 );

	// get a pointer to the backbuffer so we can check its format -
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	pD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurfaceSrc );

	D3DSURFACE_DESC	Desc;
	pSurfaceSrc->GetDesc( &Desc );
	
	RELEASE_SAFELY( pSurfaceSrc );


	// get a pointer to the depthbuffer so we can check its format -
	pD3DDevice->GetDepthStencilSurface( &pSurfaceSrc );

	D3DSURFACE_DESC	DepthDesc;
	pSurfaceSrc->GetDesc( &DepthDesc );
	
	RELEASE_SAFELY( pSurfaceSrc );

	if( m_Type == KCubeRenderTarget )
	{
		Format = D3DUSAGE_RENDERTARGET;

		HRESULT hr =  D3DXCreateRenderToEnvMap( pD3DDevice,
												m_Width,
												Desc.Format,
												kTrue,
												DepthDesc.Format,
												&m_pRenderToCubeMap );

		if ( FAILED( hr ) )
		{
			return E_FAIL;
		}
	}
	
	HRESULT hr =  D3DXCreateCubeTexture( pD3DDevice,
										 m_Width,
										 1,
										 Format,
										 Desc.Format,
										 D3DPOOL_DEFAULT,
										 &m_pTexture );
	
	// if creation of the texture failed, return
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	m_Valid = kTrue;

	return S_OK;
} // CCubeMap::Create()

void CCubeMap::CopyToFace( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						   D3DCUBEMAP_FACES Face,				// face to copy to
						   LPDIRECT3DTEXTURE8 pTexture )		// texture to copy from
{
	assert( m_Type == KCubeStandard );
	assert( m_Valid );

	IDirect3DSurface8*	pSurfaceDest		= NULL;
	IDirect3DSurface8*	pSurfaceSrc			= NULL;

	m_pTexture->GetCubeMapSurface( Face, 0, &pSurfaceDest );
	pTexture->GetSurfaceLevel( 0, &pSurfaceSrc );

	D3DSURFACE_DESC	SrcDesc;
	D3DSURFACE_DESC	DestDesc;
	pSurfaceSrc->GetDesc( &SrcDesc );
	pSurfaceSrc->GetDesc( &DestDesc );

	// the width and height of the texture we're copying MUST match the width of the cube face
	if( SrcDesc.Width != DestDesc.Width ||
		SrcDesc.Height != DestDesc.Width )
	{
		RELEASE_SAFELY( pSurfaceSrc );
		RELEASE_SAFELY( pSurfaceDest );
		return;
	}
	
	D3DXLoadSurfaceFromSurface( pSurfaceDest,
								 NULL,
								 NULL,
								 pSurfaceSrc,
								 NULL,
								 NULL,
								 D3DX_FILTER_LINEAR,
								 0 );

	RELEASE_SAFELY( pSurfaceSrc );
	RELEASE_SAFELY( pSurfaceDest );

} // CCubeMap::CopyToFace()

// create a cubemap from a dds
HRESULT CCubeMap::Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						  std::string TextureName )			// absolute path to texture
{
	m_Valid = kFalse;
	// store the path for later comparison
	m_Path = StringUpper( TextureName );

	// clean up any previous texture
	InvalidateDeviceObjects();

	m_Type = KCubeFromFile;

	// reset the material properties
	m_Valid = kFalse;

	HRESULT		hr;

	hr =  D3DXCreateCubeTextureFromFileEx( pD3DDevice,
										  TextureName.c_str(),
										  D3DX_DEFAULT,
										  1,
										  0,
										  D3DFMT_UNKNOWN,
										  D3DPOOL_DEFAULT,
										  D3DX_FILTER_LINEAR,
										  D3DX_FILTER_BOX,
										  0,
										  NULL,
										  NULL,
										  &m_pTexture );


	// if creation of the texture failed, return
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}


	// valid if completed load
	m_Valid = kTrue;

	return S_OK;
} // CCubeMap::Create()


