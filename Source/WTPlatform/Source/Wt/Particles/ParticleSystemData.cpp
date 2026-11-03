//
// ParticleSystemData.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Particles/ParticleSystemData.h"
#include "WT/Math/Math.h"
#include "WT/Image/Image.h"
#include "WT/System/IO/File.h"
#include "WT/System/Logging/Log.h"
#include <memory.h>

using namespace WT::Math;
using namespace WT::Graphics;


namespace WT
{
namespace Particles
{

WT_DEFINE_CLASS( CParticleSystemData );

CParticleSystemData::~CParticleSystemData()
{
	SafeRelease( m_pVB );
	delete[] m_pParticles;

	DeleteBehaviors();
}

//
// Accessors
//

void CParticleSystemData::SetMaxParticleCount( uint32 Count )
{
	if ( Count == m_MaxParticleCount )
		return;

	CParticle* pNewParticles = new CParticle[Count];
	if ( !pNewParticles )
		return;

	if ( Count < m_MaxParticleCount )
	{
		//we need to reduce the list of allocated particles;
		memcpy( pNewParticles, m_pParticles, Count * sizeof(CParticle) );
	}
	else
	{
		//we need to enlarge the list of allocated particles;
		memcpy( pNewParticles, m_pParticles , m_MaxParticleCount * sizeof(CParticle) );
	}

	delete[] m_pParticles;
	m_pParticles = pNewParticles;
	m_MaxParticleCount = Count;
}

CParticleBehavior* CParticleSystemData::GetBehavior( int32 ID )
{
	for (BehaviorCollection::const_iterator it=m_Behaviors.begin(); it!=m_Behaviors.end(); it++)
	{
		if ( (*it) != NULL && (*it)->GetID() == ID )
		{
			return (*it);
		}
	}

	return NULL;
}


//
// Public methods
//

void CParticleSystemData::DeleteBehaviors()
{
	m_Behaviors.clear();
}

void CParticleSystemData::Update( float32 FrameTime )
{
	//first apply any behaviors
	BehaviorCollection::iterator it = m_Behaviors.begin();
	while (it != m_Behaviors.end())
	{
		(*it)->Update( this, FrameTime );
		++it;
	}
}

void CParticleSystemData::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ) || !m_ParticleCount || pDevice == NULL )
	{
		return;
	}

	if (Context.GetRenderCategory() != ERenderCategory::PostOpaque)
	{
		return;
	}

	// Save matrices
	CMatrix4f SavedViewMatrix( false );
	pDevice->GetTransform( D3DTS_VIEW, SavedViewMatrix );

	//Since we are changing the view matrix to identity below, the clip planes
	//need to take the current view matrix into account. Only update the clip
	//planes if in use.
	DWORD ClipPlaneEnable;
	CVector4f Plane( false );
	pDevice->GetRenderState( D3DRS_CLIPPLANEENABLE, &ClipPlaneEnable );
	int32 const KPlaneIndex = 0; //Only update the first clip plane for now
	if (ClipPlaneEnable & (1 << KPlaneIndex))
	{
		pDevice->GetClipPlane( KPlaneIndex, Plane.e );

		CMatrix4f WorldMatrix( false );
		pDevice->GetTransform( D3DTS_WORLD, WorldMatrix );
		CMatrix4f WorldToViewMatrix( WorldMatrix * SavedViewMatrix );

		CVector4f NewPlane( false );
		WorldToViewMatrix.RotateNoOverlap( NewPlane, Plane );
		NewPlane.w = Plane.w - NewPlane.CVector3f::DotProduct( WorldToViewMatrix.RowT );

		pDevice->SetClipPlane( KPlaneIndex, NewPlane.e );
	}

	pDevice->SetFVF( P_FVF );
	pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( Vertex ) );

	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, m_FlipbookAnimation ? D3DTTFF_COUNT2 : D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );

	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	if (m_Additive)
	{
		pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	}
	else
	{
		pDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	//Use identity world matrix
	pDevice->SetTransform( D3DTS_VIEW, CMatrix4f::gm_Identity.GetD3DMatrixPtr() );

	CMatrix4f UVMatrix;
	int32 PrevFrame = -1;

	for (uint32 i = 0; i < m_ParticleCount; ++i)
	{
		CParticle const& Particle = m_pParticles[i];

		if (m_FlipbookAnimation)
		{
			int32 const Frame = Float32ToInt32( floor( Particle.Frame ) );
			if (Frame != PrevFrame)
			{
				uint32 const Row = Frame / m_FramesPerWidth;
				uint32 const Column = Frame - Row * m_FramesPerWidth;

				UVMatrix.RowZ.x = (float32) Column / m_FramesPerWidth;
				UVMatrix.RowZ.y = (float32) Row / m_FramesPerHeight;

				pDevice->SetTransform( D3DTS_TEXTURE0 , UVMatrix );
				PrevFrame = Frame;
			}
		}

		uint32 ParticleColor = D3DCOLOR_ARGB(
			Float32ToInt32( Particle.Opacity ),
			Float32ToInt32( Particle.Color.x ),
			Float32ToInt32( Particle.Color.y ),
			Float32ToInt32( Particle.Color.z ) );
		pDevice->SetRenderState( D3DRS_TEXTUREFACTOR , ParticleColor );

		CMatrix4f Transform( false );
		Transform.SetZRotation( Particle.Rotation.z );
		Transform.PreScale( Particle.Size );

		if (m_OrientedParticles)
		{
			CVector3f const& ParticleOrigin( CVector3f::gm_Zero );
			CVector3f const& ParticleUp( CVector3f::gm_YAxis );
			CMatrix4f OrientationMatrix( false );
			OrientationMatrix.SetLookAt( ParticleOrigin, Particle.Velocity, ParticleUp );

			Transform.Multiply3x3( Transform, OrientationMatrix );
			Transform.PostTranslate( Particle.Position );
			Transform *= SavedViewMatrix;
		} else {
			SavedViewMatrix.TransformNoOverlap( Transform.RowT, Particle.Position );
		}

		pDevice->SetTransform( D3DTS_WORLD, Transform );

		int32 PassCount;
		m_hMaterial->Begin( pDevice, Context, PassCount );
		for (int32 i = 0; i < PassCount; ++i)
		{
			m_hMaterial->BeginPass( pDevice, Context, i );
			pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
			m_hMaterial->EndPass();
		}
		m_hMaterial->End();
	}

	// Restore matrices
	pDevice->SetTransform( D3DTS_VIEW, SavedViewMatrix );

	// Restore clip planes if in use
	if (ClipPlaneEnable & (1 << KPlaneIndex))
	{
		pDevice->SetClipPlane( KPlaneIndex, Plane.e );
	}

	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}


//
// ISerializable
//

void CParticleSystemData::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
										System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "MaxParticleCount", m_MaxParticleCount );
	pInfo->AddValue( "ParticleCount", m_ParticleCount );
	pInfo->AddValue( "FrameWidth", m_FrameWidth );
	pInfo->AddValue( "FrameHeight", m_FrameHeight );
	pInfo->AddValue( "FramesPerWidth", m_FramesPerWidth );
	pInfo->AddValue( "FramesPerHeight", m_FramesPerHeight );
	pInfo->AddValue( "FlipbookAnimation", m_FlipbookAnimation );
	pInfo->AddValue( "Additive", m_Additive );
	pInfo->AddValue( "OrientedParticles", m_OrientedParticles );
	
	// Particles
	pInfo->AddContainerBeginWithCount( "Particles", m_ParticleCount );
	for (uint32 i=0; i<m_ParticleCount; ++i)
	{
		pInfo->AddValue( "Particle", m_pParticles[i] );
	}
	pInfo->AddContainerEnd();

	// Particle behaviors
	pInfo->AddContainerBeginWithCount( "Behaviors", static_cast<uint32>(m_Behaviors.size()) );
	for (BehaviorCollection::const_iterator it=m_Behaviors.begin(); it!=m_Behaviors.end(); it++)
	{
		pInfo->AddValue( "Behavior", (*it).get() );
	}
	pInfo->AddContainerEnd();

	// Save filename for particle texture
	string TextureName;
	if (m_hMaterial != NULL)
	{
		TextureName = m_hMaterial->GetTextureFilename();
	}
	pInfo->AddValue( "TextureName", TextureName );
}

void CParticleSystemData::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
										System::Serialization::CSerializationContext* pContext )
{
	// Get device context from serialization context
	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( pContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );

	uint32 MaxParticleCount;
	pInfo->GetValue( "MaxParticleCount", MaxParticleCount );
	SetMaxParticleCount( MaxParticleCount );

	pInfo->GetValue( "ParticleCount", m_ParticleCount );
	pInfo->GetValue( "FrameWidth", m_FrameWidth );
	pInfo->GetValue( "FrameHeight", m_FrameHeight );
	pInfo->GetValue( "FramesPerWidth", m_FramesPerWidth );
	pInfo->GetValue( "FramesPerHeight", m_FramesPerHeight );

	bool b;
	pInfo->GetValue( "FlipbookAnimation", b ); m_FlipbookAnimation = b;
	pInfo->GetValue( "Additive", b ); m_Additive = b;
	pInfo->GetValue( "OrientedParticles", b ); m_OrientedParticles = b;

	// Particles
	uint32 Count = pInfo->GetContainerBeginWithCount( "Particles" );
	if ( Count > 0 )
	{
		m_pParticles = new CParticle[Count];
		for (uint32 i=0; i<Count; ++i)
		{
			pInfo->GetValue( "Particle", m_pParticles[i] );
		}
	}
	pInfo->GetContainerEnd();

	// Particle behaviors
	Count = pInfo->GetContainerBeginWithCount( "Behaviors" );
	m_Behaviors.resize( Count );
	for (uint32 i=0; i<Count; i++)
	{
		CObject* pObject;
		pInfo->GetValue( "Behavior", pObject );
		CParticleBehavior* pBehavior = DynamicCast<CParticleBehavior>( pObject );
		if ( pBehavior == NULL )
		{
			throw CParticleException( "CParticleSystemData::SetObjectData - NULL ParticleBehavior object pointer from serialization stream" );
		}
		m_Behaviors[i] = pBehavior;
	}
	pInfo->GetContainerEnd();

	// Textured material
	string TextureName;
	pInfo->GetValue( "TextureName", TextureName );
	if (TextureName.size())
	{
		std::string TexturePath = TextureName;
		if (!System::IO::CFile::Exists( TexturePath ))
		{
			std::vector< string > const& TexturePaths = 
				pContext->GetProperties()->GetValue< std::vector< string > >( "TexturePaths", std::vector< string >() );
			std::vector< string >::const_iterator it = TexturePaths.begin();
			for (; it != TexturePaths.end(); ++it)
			{
				if (System::IO::CFile::Exists( *it + TextureName ))
				{
					TexturePath = *it + TextureName;
					break;
				}
			}

			if (it == TexturePaths.end())
			{
				LOG_PURPLE( "Could not find '%s' in TexturePaths.", TextureName.c_str() );
			}
		}

		m_hMaterial = new CParticleMaterial( pDevice, TexturePath );
	}

	CreateVB( pDevice );
}


// Private methods

void CParticleSystemData::CreateVB( LPDIRECT3DDEVICE9 pDevice )
{
	SafeRelease(m_pVB);
	HRESULT hr = pDevice->CreateVertexBuffer( (4 * sizeof(Vertex)), 0, P_FVF, D3DPOOL_MANAGED, &m_pVB, NULL );

	if (!SUCCEEDED(hr))
	{
		assert( m_pVB == NULL );
		WT_THROW( System::CRuntimeException, "CreateVertexBuffer failure" );
	}

	Vertex* pVertex;
	hr = m_pVB->Lock( 0, 4 * sizeof(Vertex), reinterpret_cast<void**>(&pVertex), 0 );
	if (!SUCCEEDED(hr))
	{
		m_pVB->Release();
		m_pVB = NULL;
		WT_THROW( System::CRuntimeException, "Lock failure" );
	}

	float32 FrameWidth = 1.0f;
	float32 FrameHeight = 1.0f;

	if ( m_FlipbookAnimation )
	{
		int32 TexWidth, TexHeight;
		if ( m_hMaterial->GetTextureSize(0, TexWidth, TexHeight) )
		{
			FrameWidth = (float32) m_FrameWidth / (float32) TexWidth;
			FrameHeight = (float32) m_FrameHeight / (float32) TexHeight;

			m_FramesPerWidth = TexWidth / m_FrameWidth;
			m_FramesPerHeight = TexHeight / m_FrameHeight;
		}
		else
		{
			assert(0);
			m_FlipbookAnimation = false;
			m_FramesPerWidth = 1;
			m_FramesPerHeight = 1;
		}
	}
	else
	{
		m_FramesPerWidth = 1;
		m_FramesPerHeight = 1;
	}

	pVertex[0].SetPos( -0.5f, 0.5f, 0.0f );
	pVertex[0].diffuse = 0xffffffff;
	pVertex[0].SetTex( 0.0f, 0.0f );

	pVertex[1].SetPos( 0.5f, 0.5f, 0.0f );
	pVertex[1].diffuse = 0xffffffff;
	pVertex[1].SetTex( FrameWidth, 0.0f );

	pVertex[2].SetPos( -0.5f, -0.5f, 0.0f );
	pVertex[2].diffuse = 0xffffffff;
	pVertex[2].SetTex( 0.0f, FrameHeight );

	pVertex[3].SetPos( 0.5f, -0.5f, 0.0f );
	pVertex[3].diffuse = 0xffffffff;
	pVertex[3].SetTex( FrameWidth, FrameHeight );

	m_pVB->Unlock();
}


// CParticleMaterial class methods

void CParticleMaterial::LoadTextureFromFile( IDirect3DDevice9* pDevice, const string& TexturePath )
{
	assert( pDevice );

	// Extract filename from Path string and save
	size_t Index = TexturePath.find_last_of('\\');
	if ( Index != std::string::npos )
	{
		++Index;
		m_TextureFilename = TexturePath.substr( Index, (TexturePath.size() - Index) );
	}
	else
	{
		m_TextureFilename = TexturePath;
	}

	// Load texture image
	HImage hImage = CImage::CreateImage( TexturePath );
	if (!hImage)
	{
		throw CParticleException( "CParticleMaterial::LoadTextureFromFile - Load texture image from file failed" );
	}

	// Create D3D device texture
	IDirect3DBaseTexture9* pTexture = hImage->CreateD3DTexture( pDevice );
	if ( !pTexture )
	{
		throw CParticleException( "CParticleMaterial::LoadTextureFromFile - Create D3D texture from image failed" );
	}

	SetTexture( pTexture );
	pTexture->Release();
}


} // namespace Particles
} // namespace WT
