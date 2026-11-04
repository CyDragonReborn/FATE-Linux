/*
	material.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/constants.h"
#include "materialconstants.h"

// forward declarations
class CMovie;
class CTexture;
class CRefManager;
class CCubeMap;

const uint32	KMaxLayers	=	3;

using namespace std;
// the material class handles loading of various texture types, assembly of
// textures with opacity masks from separate textures, and animation properties, like
// shifting and flipbooks.
class CMaterial
{
public:
						CMaterial( CRefManager& pRefManager,					// reference manager (primarily for use in preventing duplicate movies)
								   bool AllowTextureCompression = kTrue );		// allow DXT texture compression?
						~CMaterial( void );

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								const std::string& TextureName,		// absolute path to texture
								bool ColorKey = kFalse,				// use colorkey?
								DWORD ColorKeyValue = D3DCOLOR_ARGB( 255, 255, 0, 255 ) );	// defaults to magenta, if present

	HRESULT				Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								std::string TextureName,			// absolute path to base texture
								const std::string& OpacityName,		// absolute path to opacity mask
								int32 MipLevels = D3DX_DEFAULT );	// # of mip levels to generate (D3DX_DEFAULT is default)

	HRESULT				CreateIllumination( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
											const std::string& TextureName );	// absolute path to texture

	HRESULT				CreateNormalMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
										 const std::string& TextureName );	// absolute path to texture

	HRESULT				CreateSphereMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
										 const std::string& TextureName );	// absolute path to texture

	HRESULT				CreateCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									   const std::string& TextureName );	// absolute path to texture

	void				Update( float32 TimeElapsed );		// seconds elapsed	

	void				DisableAlpha( void )		{	m_HasAlpha = kFalse;		};

	void				EnableDynamicTexture( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
											  uint32 Width,						// pixel width of DynamicTexture
											  uint32 Height = 0 );				// pixel height of DynamicTexture ( default value will use width )

	HRESULT				BeginRender( void );
	void				EndRender( void );

	void				SetDynamicTexturePlane( const D3DXVECTOR3& CenterPoint, 
										    const D3DXVECTOR3& Normal );


	void				InvalidateDeviceObjects( void );
	void				RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void				ResetAnimationProperties( void );

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	uint32				DynamicTextureWidth( void )			{	return m_DynamicTextureWidth;		};
	uint32				DynamicTextureHeight( void )		{	return m_DynamicTextureHeight;		};

	// conversion factors from pixel-space to UV space
	float32				PixelToU( void )					{	return 1.0f / Width();	};
	float32				PixelToV( void )					{	return 1.0f / Height();	};

	bool				BlankBaseTexture( void )			{	return m_BlankBaseTexture;		};

	bool				TransparentReflection( void )		{	return m_TransparentReflection;	};

	bool				AdditiveReflection( void )			{	return m_AdditiveReflection;	};

	D3DMATERIAL8&		D3DMaterial( void )					{	return m_D3DMaterial;			};

	const D3DXPLANE&	DynamicTexturePlane( void )			{	return m_DynamicTexturePlane;		};
	const D3DXVECTOR3&	DynamicTextureNormal( void )		{	return m_DynamicTextureNormal;		};

	EMaterialType		GetIlluminationType( void )			{	return m_IlluminationType;		};

	bool				IsAnimated( uint32 Layer );		// layer to check animation properties for

	HRESULT				ConvertToFormatSystem( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
											   D3DFORMAT D3DFormat );				// desired format to convert to	

	LPDIRECT3DTEXTURE8	GetTexture( void );

	LPDIRECT3DTEXTURE8	GetIlluminationTexture( void );

	LPDIRECT3DTEXTURE8	GetNormalTexture( void );

	LPDIRECT3DTEXTURE8	GetAlphaTexture( void );

	LPDIRECT3DTEXTURE8	GetSphereMapTexture( void );

	LPDIRECT3DTEXTURE8	GetDynamicTexture( void );

	CCubeMap*			GetCubeMap( void );

	uint32				Width( void );
	uint32				Height( void );

	bool				IsValid( void )					{	return m_Valid;				};
	bool				IsDynamicTextureValid( void )	{	return m_DynamicTextureValid;	};
	
	bool				HasAlpha( void )			{	return m_HasAlpha;			};
	
	bool				HasColorKey( void )			{	return m_HasColorKey;		};
	
	bool				HasIllumination( void )		{	return m_HasIllumination;	};

	bool				HasNormalMap( void )		{	return m_HasNormalMap;		};

	bool				HasReflection( void )		{	return m_HasReflection;		};
	
	bool				HasBaseTexture( void )		{	return m_HasBaseTexture;	};	

	bool				HasAlphaTexture( void )		{	return m_HasAlphaTexture;	};	

	bool				HasSphereMap( void )		{	return m_HasSphereMap;		};	

	bool				HasCubeMap( void )			{	return m_HasCubeMap;		};	

	float32				GetU( uint32 Layer )		{	return m_ShiftPositionU[Layer];	};

	float32				GetV( uint32 Layer )		{	return m_ShiftPositionV[Layer];	};

	const std::string&	GetPath( void )					{	return m_Path;					};
	const std::string&	GetAlphaPath( void )			{	return m_AlphaPath;				};
	const std::string&	GetIlluminationMapPath( void )	{	return m_IlluminationMapPath;	};
	const std::string&	GetNormalMapPath( void )		{	return m_NormalMapPath;			};
	const std::string&	GetSphereMapPath( void )		{	return m_SphereMapPath;			};
	const std::string&	GetCubeMapPath( void )			{	return m_CubeMapPath;			};

	uint32				GetDataID( void )			{	return m_DataID;			};

	void				SetDataID( uint32 Id )		{	m_DataID = Id;			};

	bool				AllowZWrite( void )			{	return m_AllowZWrite;	};

	////////////////////////////////////////////////////////////////////////////////////////////
	// MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void				SetAllowZWrite( bool State )	{	m_AllowZWrite = State;	};

	void				SetIlluminationType( EMaterialType Type )		{	m_IlluminationType = Type;			};

	void				SetD3DMaterial( const D3DMATERIAL8& Material )	{	m_D3DMaterial = Material;			};

	void				SetTransparentReflection( bool State )			{	m_TransparentReflection = State;	};

	void				SetAdditiveReflection( bool State )				{	m_AdditiveReflection = State;		};

	void				SetAnimationShifting( uint32 Layer,	// layer to perform operation on
											  float32 U,	// how far to scroll in U per second
											  float32 V );	// how far to scroll in V per second

	void				SetAnimationFlipbook( uint32 Layer,				// layer to perform operation on
											  int32 FrameWidth,			// frames wide
											  int32 FrameHeight,		// frames high
											  float32 FrameTime );		// how many seconds until a frame flips

	void				SetReflective( bool State );			// reflective state

	void				SetNormalMapped( bool State );			// reflective state

	void				SetAnimationStates( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void				SetAnimationLayerStates( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
												 uint32 Layer );				// layer to set states for

	void				UnSetAnimationStates( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void				UnSetAnimationLayerStates( LPDIRECT3DDEVICE8 pD3DDevice,// direct3d device
												   uint32 Layer );				// layer to set states for
private:
	CRefManager&		m_pRefManager;

	D3DMATERIAL8		m_D3DMaterial;

	std::string			m_Path;	
	std::string			m_AlphaPath;	
	std::string			m_IlluminationMapPath;
	std::string			m_NormalMapPath;
	std::string			m_SphereMapPath;
	std::string			m_CubeMapPath;

	EMaterialType		m_IlluminationType;	// is our illumination map a multiply or an add? default add

	uint32				m_DataID;		// user data that is loaded from model files

	uint32				m_DynamicTextureWidth;
	uint32				m_DynamicTextureHeight;

	bool				m_AllowZWrite;

	bool				m_Valid;
	bool				m_DynamicTextureValid;
	bool				m_HasAlpha;
	bool				m_HasColorKey;

	bool				m_TransparentReflection;	// is reflective AND transparent
	bool				m_AdditiveReflection;		// is a self-illuminated reflection

	bool				m_HasIllumination;
	bool				m_HasReflection;
	bool				m_HasDynamicTexture;
	bool				m_HasAlphaTexture;
	bool				m_HasNormalMap;
	bool				m_HasSphereMap;
	bool				m_HasCubeMap;
	bool				m_HasBaseTexture;
	bool				m_BlankBaseTexture;	// a base texture was not supplied, 
											// so a blank one was made

	// layer animation properties
	bool				m_FlipbookAnimation[KMaxLayers];
	bool				m_ShifterAnimation[KMaxLayers];
	float32				m_ShiftSpeedU[KMaxLayers];
	float32				m_ShiftSpeedV[KMaxLayers];
	float32				m_ShiftPositionU[KMaxLayers];
	float32				m_ShiftPositionV[KMaxLayers];
	float32				m_AnimationTimer[KMaxLayers];
	float32				m_FrameTime[KMaxLayers];
	float32				m_FrameHeight[KMaxLayers];
	float32				m_FrameWidth[KMaxLayers];
	uint32				m_FramesWide[KMaxLayers];
	uint32				m_FramesHigh[KMaxLayers];
	uint32				m_CurrentFrame[KMaxLayers];


	CCubeMap*			m_pCubeMap;

	CTexture*			m_pTexture;
	CTexture*			m_pAlphaTexture;
	CTexture*			m_pIlluminationTexture;
	CTexture*			m_pNormalTexture;
	CTexture*			m_pSphereMapTexture;

	CMovie*				m_pMovie;
	CMovie*				m_pIlluminationMovie;

	// our DynamicTexture map objects - these allow us to render to the texture
	ID3DXRenderToSurface*	m_pRenderToDynamicTextureMap;
	LPDIRECT3DTEXTURE8		m_pDynamicTextureMap;

	D3DXPLANE			m_DynamicTexturePlane;
	D3DXVECTOR3			m_DynamicTextureNormal;

	bool				m_AllowTextureCompression;
};

#endif