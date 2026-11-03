//
// ParticleSystemData.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The CParticleSystemData class is the main class for holding a collection of CParticle objects, updating CParticle
//	attributes (through Update() method), and rendering each CParticle object.  Particles can be rendered with 
//	textures or a series of textures over time (if in Flipbook animation mode).  Textures are applied through the
//	CParticleMaterial class that holds the texture as well as the pre-render texture stage set up.  Rendering can be 
//	additive or blended based on the Additive attribute.  
//	This class also keeps a collection of particle behaviors.  Particles are updated (through Update() method) by 
//	executing each ParticleBehavior's Update() method in the behavior collection.
//
//	Note that this class "owns" all behavior, particle, and material objects and will delete them on class destruction.  
//	Specific behavior object pointers can be looked up and returned through the GetBehavior() method, based on its 
//	unique ID.
//

#pragma once

#include <vector>
#include "d3d9.h"
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/Material.h"
#include "WT/Particles/Particle.h"
#include "WT/Particles/ParticleBehavior.h"
#include "WT/Particles/ParticleException.h"

namespace WT
{
namespace Particles
{

typedef std::vector<HParticleBehavior> BehaviorCollection;

// Material class for the Particle system
typedef System::SmartPointer<class CParticleMaterial> HParticleMaterial;
class CParticleMaterial : public Graphics::CMaterialTextured
{
public:
	WT_CLASS( CParticleMaterial, Graphics::CMaterial )

	CParticleMaterial() {}
	CParticleMaterial( IDirect3DBaseTexture9* pTexture) : CMaterialTextured( pTexture ) {}
	CParticleMaterial( IDirect3DDevice9* pDevice, const string& TexturePath )
	{
		LoadTextureFromFile( pDevice, TexturePath );
	}
	~CParticleMaterial() {}

	// Accessors
	const string& GetTextureFilename() const
	{
		return m_TextureFilename;
	}
	void SetTextureFilename( const string& TextureFilename )
	{
		m_TextureFilename = TextureFilename;
	}


	void LoadTextureFromFile( IDirect3DDevice9* pDevice, const string& TexturePath );

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, Graphics::CRenderContext const& /*Context*/, int32 PassIndex ) const
	{
		assert( pDevice != NULL );

		if (PassIndex < 0 || PassIndex >= 1)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
		}

		pDevice->SetTexture( 0, m_pTexture );
        pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
        pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
        pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
        pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
        pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
		
		/*CParticleSystemData::Render() takes care of 
		D3DTSS_TEXCOORDINDEX and D3DTSS_TEXTURETRANSFORMFLAGS for stage 0.
		This enables particle flipbooking.*/

        pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}

private: // data
	string	m_TextureFilename;
};

typedef System::SmartPointer<class CParticleSystemData> HParticleSystemData;
class CParticleSystemData : public System::CObject, public System::Serialization::ISerializable, public Graphics::CRenderableImpl,
	public Graphics::IUpdateable
{
public:
	
	WT_BEGIN_CLASS( CParticleSystemData, System::CObject )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( Graphics::IRenderable );
		WT_INHERITS_INTERFACE( Graphics::IUpdateable );
	WT_END_CLASS()

	CParticleSystemData()
		: m_pParticles( NULL )
		, m_pVB( NULL )
	{
		InitMembers();
	}

	CParticleSystemData( IDirect3DDevice9* pDevice, CParticleMaterial* pMaterial, 
						uint32 FrameWidth = 0, uint32 FrameHeight = 0 )
		: m_pParticles(NULL)
		, m_pVB(NULL)
	{
		InitMembers();
		Init( pDevice, pMaterial, FrameWidth, FrameHeight );
	}
		
	virtual ~CParticleSystemData();

	void Init( LPDIRECT3DDEVICE9 pDevice, CParticleMaterial* pMaterial, uint32 FrameWidth = 0, uint32 FrameHeight = 0 )
	{
		m_hMaterial = pMaterial;
		m_FrameWidth = FrameWidth;
		m_FrameHeight = FrameHeight;
		m_FlipbookAnimation = (FrameWidth==0 || FrameWidth==0) ? false : true;

		// Must have a valid material
		if ( !m_hMaterial )
		{
			throw CParticleException( "CParticleSystemData::CParticleSystemData - Invalid material pointer" );
		}

		CreateVB( pDevice );
	}


	//
	// Accessors
	//
		
	void SetMaxParticleCount( uint32 Count );
	uint32 GetMaxParticleCount () const { return m_MaxParticleCount; }

	void SetParticleCount( uint32 Count ) { m_ParticleCount = Count; }
	uint32 GetParticleCount() const { return m_ParticleCount; }

	CParticle* GetParticle( uint32 Index )
	{
		if (Index >= 0 && Index < m_ParticleCount)
			return &(m_pParticles[Index]);

		return NULL;
	}
	bool RemoveParticle( uint32 Index )
	{
		if ( Index < 0 || Index >= m_ParticleCount )
			return false;

		m_pParticles[Index] = m_pParticles[--m_ParticleCount];
		return true;
	}
	bool AddParticle( const CParticle& Particle )
	{
		if ( m_ParticleCount >= m_MaxParticleCount )
			return false;

		m_pParticles[m_ParticleCount++] = Particle;
		return true;
	}

	CParticle* GetParticles() { return m_pParticles; }

	bool GetFlipbookAnimation() const { return m_FlipbookAnimation; }

	uint32 GetFrameWidth() const { return m_FrameWidth; }

	uint32 GetFrameHeight() const { return m_FrameHeight; }

	void SetAdditive( bool Additive ) { m_Additive = Additive; }
	bool GetAdditive() const { return m_Additive; }

	void SetOrientedParticles( bool OrientedParticles ) { m_OrientedParticles = OrientedParticles; }
	bool GetOrientedParticles() const { return m_OrientedParticles; }

	void SetMaterial( CParticleMaterial* pMaterial )
	{
		m_hMaterial = pMaterial;
	}

	CParticleMaterial* GetMaterial() const
	{
		return m_hMaterial;
	}

	BehaviorCollection& GetBehaviors()
	{
		return m_Behaviors;
	}

	CParticleBehavior* GetBehavior( int32 ID );

	void DeleteBehaviors();

	// Render the system using the specified device.
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, Graphics::CRenderContext const& Context );

	virtual void Update( float32 ElapsedTimeSeconds );

	// Static creation method
	static HParticleSystemData Create( IDirect3DDevice9* pDevice, CParticleMaterial* pMaterial, 
									   uint32 FrameWidth = 0, uint32 FrameHeight = 0 )
	{
		return new CParticleSystemData( pDevice, pMaterial, FrameWidth, FrameHeight );
	}


	//
	// ISerializable interface
	//

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );


private: // methods

	void InitMembers()
	{
		m_MaxParticleCount = 0;
		m_ParticleCount = 0;

		m_FrameWidth = 0;
		m_FrameHeight = 0;
		m_FramesPerWidth = 1;
		m_FramesPerHeight = 1;

		m_FlipbookAnimation = false;
		m_Additive = false;
		m_OrientedParticles = false;
	}

	void CreateVB( LPDIRECT3DDEVICE9 pDevice );


private: // data

	uint32						m_MaxParticleCount;
	uint32						m_ParticleCount;
	uint32						m_FrameWidth;
	uint32						m_FrameHeight;
	uint32						m_FramesPerWidth;
	uint32						m_FramesPerHeight;
	CParticle*					m_pParticles;
	BehaviorCollection			m_Behaviors;
	IDirect3DVertexBuffer9*		m_pVB;
	HParticleMaterial			m_hMaterial;

	bool						m_FlipbookAnimation : 1;
	bool						m_Additive			: 1;
	bool						m_OrientedParticles : 1;

	// Typedefs
	struct Vertex
	{
		float32		x, y, z;		// Position
		uint32		diffuse;		// Diffuse color
		float32		u, v;			// Texture coordinates

		void SetPos(float32 fx, float32 fy, float32 fz) { x=fx; y=fy; z=fz; }
		void SetTex(float32 fu, float32 fv) { u=fu; v=fv; }
	};

	static const uint32 P_FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
};

} //namespace Particles
} //namespace WT
