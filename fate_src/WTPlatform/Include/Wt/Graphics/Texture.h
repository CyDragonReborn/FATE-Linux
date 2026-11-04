/***************************************************************************************
 *
 * Texture.h
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

#pragma once

#include "d3d9.h"
#include "WT/System/Object.h"
#include "WT/System/IInterface.h"
#include "WT/Graphics/AVITexture.h"
#include "WT/Image/Image.h"

namespace WT
{
namespace Graphics
{

//Global simulation time; used for all graphics-related animation effects such as movie textures
//and scrolling or flipbook materials.
extern float64 gSimulationTimeSeconds;

//Forward declarations
typedef System::SmartPointer<class CSimpleTexture> HSimpleTexture;

//Texture caching - used to avoid producing duplicate D3D textures
//corresponding to the same image files or combination of diffuse and
//opacity image files.
struct TexturePaths
{
	string Diffuse;
	string Opacity;

	bool operator<( TexturePaths const& other ) const
	{
		if (Diffuse < other.Diffuse) return true;
		if (Diffuse > other.Diffuse) return false;
		if (Opacity < other.Opacity) return true;
		return false;
	}
};

//An interface that provides access to a D3D texture.
typedef System::SmartPointer<class CTexture> HTexture;
class CTexture : public System::CObject, public IDeviceObject
{
public:
	WT_BEGIN_ABSTRACT_CLASS( CTexture, System::CObject );
		WT_INHERITS_INTERFACE( IDeviceObject );
	WT_END_CLASS();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	virtual LPDIRECT3DBASETEXTURE9 GetD3DTexture() = 0;

	static HTexture WTCall Load( string const& Path, LPDIRECT3DDEVICE9 pDevice, bool bOpacity = false, HImage* phImage = NULL );

	//Cached textures
	static void WTCall FlushTextureCache();

	typedef std::map< std::string, HSimpleTexture > TextureCache;
	typedef std::map< TexturePaths, HSimpleTexture > CombinedCache; //Combined diffuse + opacity textures
	typedef std::map< std::string, HTexture > MovieCache;

	static TextureCache gm_TextureCache;
	static CombinedCache gm_CombinedCache;
	static MovieCache gm_MovieCache;
};

typedef System::SmartPointer<class CSimpleTexture> HSimpleTexture;
class CSimpleTexture : public CTexture
{
public:
	WT_CLASS( CSimpleTexture, CTexture )

	CSimpleTexture()
		: m_pTexture( NULL )
	{
	}

	CSimpleTexture( CSimpleTexture const& texture )
	{
		if (texture.m_pTexture) texture.m_pTexture->AddRef();
		m_pTexture = texture.m_pTexture;
	}

	~CSimpleTexture()
	{
		SafeRelease( m_pTexture );
	}

	CSimpleTexture& operator=( CSimpleTexture const& texture )
	{
		if (texture.m_pTexture) m_pTexture->AddRef();
		SafeRelease( m_pTexture );
		m_pTexture = texture.m_pTexture;
		return *this;
	}

	virtual LPDIRECT3DBASETEXTURE9 GetD3DTexture()
	{
		return m_pTexture;
	}

	void SetD3DTexture( LPDIRECT3DBASETEXTURE9 pTexture )
	{
		if (pTexture)
		{
			pTexture->AddRef();
		}
		SafeRelease( m_pTexture );
		m_pTexture = pTexture;
	}

private:
	LPDIRECT3DBASETEXTURE9 m_pTexture;
};

typedef System::SmartPointer<class CMovieTexture> HMovieTexture;
class CMovieTexture : public CTexture
{
public:
	WT_CLASS_NO_CREATOR( CMovieTexture, CTexture )

	CMovieTexture( std::string const& Path, LPDIRECT3DDEVICE9 pDevice, bool bLooping = true );

	virtual LPDIRECT3DBASETEXTURE9 GetD3DTexture();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

private:
	float64 m_SimulationTimeSeconds;
	CAVITexture m_AVITexture;
};

}
}
