/*
	cubemap.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	4/21/2004

*/

#ifndef _CUBEMAP_H
#define _CUBEMAP_H

#include <D3D8.h>
#include <d3dx8.h>

class CMaterial;

#include "../UTILITIES/constants.h"

using namespace std;

enum ECubeMapType
{
	KCubeFromFile,
	KCubeFromFiles,
	KCubeStandard,
	KCubeRenderTarget
};

// cubemaps can be created by loading a .dds directly, or can be assembled from 6
// separate textures
class CCubeMap
{
public:
						CCubeMap( void );
						~CCubeMap( void );

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								std::string TextureName );			// absolute path to texture

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								uint32 Width,						// width of cube
								ECubeMapType Type );				// render target or standard

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
								LPDIRECT3DTEXTURE8 pRight,		// texture to copy from
								LPDIRECT3DTEXTURE8 pLeft,		// texture to copy from
								LPDIRECT3DTEXTURE8 pUp,			// texture to copy from
								LPDIRECT3DTEXTURE8 pDown,		// texture to copy from
								LPDIRECT3DTEXTURE8 pFront,		// texture to copy from
								LPDIRECT3DTEXTURE8 pBack );		// texture to copy from	

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
								CMaterial& pRight,				// texture to copy from
								CMaterial& pLeft,				// texture to copy from
								CMaterial& pUp,					// texture to copy from
								CMaterial& pDown,				// texture to copy from
								CMaterial& pFront,				// texture to copy from
								CMaterial& pBack );				// texture to copy from	

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device 
								const std::string& Right,		// path to copy from
								const std::string& Left,		// path to copy from
								const std::string& Up,			// path to copy from
								const std::string& Down,		// path to copy from
								const std::string& Front,		// path to copy from
								const std::string& Back );		// path to copy from
	
	HRESULT				BeginRender( void );

	HRESULT				SetRenderFace( D3DCUBEMAP_FACES Face );	// face to begin a render on

	void				EndRender( void );

	void				CopyToFace( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
									D3DCUBEMAP_FACES Face,				// face to copy to
									LPDIRECT3DTEXTURE8 pTexture );		// texture to copy from

	bool				IsValid( void )				{	return m_Valid;				};
	
	std::string			GetPath( void )				{	return m_Path;				};

	LPDIRECT3DCUBETEXTURE8	GetTexture( void )		{	return m_pTexture;			};

	void				InvalidateDeviceObjects( void );
	void				RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	ECubeMapType		Type( void )				{	return m_Type;		};

private:
	LPDIRECT3DCUBETEXTURE8	m_pTexture;
	ID3DXRenderToEnvMap*	m_pRenderToCubeMap;

	std::string				m_Path;	
	std::string				m_RightPath;	
	std::string				m_LeftPath;	
	std::string				m_UpPath;	
	std::string				m_DownPath;	
	std::string				m_FrontPath;	
	std::string				m_BackPath;	

	uint32					m_Width;

	bool					m_Valid;

	ECubeMapType			m_Type;
};

#endif