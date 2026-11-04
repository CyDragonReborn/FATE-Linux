/***************************************************************************************
 *
 * MaterialMap.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          05/3/2006
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Graphics/MaterialMap.h"

using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CMaterialMap )

void CMaterialMap::UpdateTransform()
{
	m_Transform.SetEulerAngles( m_Rotation );
	m_Transform.PreScale( m_Scale.x, m_Scale.y, m_Scale.z );
	m_Transform.PreTranslate( m_Translation );
	m_bUseTransform = (m_Transform == CMatrix4f::gm_Identity) ? false : true;
}

CMaterialMap::CMaterialMap() 
	: m_Scale( CVector3f::gm_UnitScale )
	, m_MapType( EMaterialMap::KExplicit )
	, m_MapChannel( 0 )
	, m_ResourcesValid( false )
	, m_bUseTransform( false )
	, m_bUseLuminanceAsAlpha( false )
{
	m_bTile[0] = m_bTile[1] = m_bTile[2] = true;
	m_bMirror[0] = m_bMirror[1] = m_bMirror[2] = false;
}

CMaterialMap::CMaterialMap( CMaterialMap const& MaterialMap )
	: m_Name( MaterialMap.m_Name )
	, m_hTexture( MaterialMap.m_hTexture )
	, m_Scale( MaterialMap.m_Scale )
	, m_Rotation( MaterialMap.m_Rotation )
	, m_Translation( MaterialMap.m_Translation )
	, m_Transform( MaterialMap.m_Transform )
	, m_MapType( MaterialMap.m_MapType )
	, m_MapChannel( MaterialMap.m_MapChannel )
	, m_ResourcesValid( MaterialMap.m_ResourcesValid )
	, m_bUseTransform( MaterialMap.m_bUseTransform )
	, m_bUseLuminanceAsAlpha( MaterialMap.m_bUseLuminanceAsAlpha )
{
	m_bTile[0] = MaterialMap.m_bTile[0];
	m_bTile[1] = MaterialMap.m_bTile[1];
	m_bTile[2] = MaterialMap.m_bTile[2];
	m_bMirror[0] = MaterialMap.m_bMirror[0];
	m_bMirror[1] = MaterialMap.m_bMirror[1];
	m_bMirror[2] = MaterialMap.m_bMirror[2];
}

CMaterialMap::~CMaterialMap()
{
}

void CMaterialMap::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;

	if (m_hTexture != NULL)
	{
		m_hTexture->InvalidateDeviceObjects();
	}
}

void CMaterialMap::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	if (m_hTexture != NULL)
	{
		m_hTexture->RestoreDeviceObjects( pDevice );
	}

	m_ResourcesValid = true;
}

bool CMaterialMap::IsValid() const
{
	return m_ResourcesValid;
}

bool CMaterialMap::IsCompatible( CMaterialMap const& Map ) const
{
	return 
		m_MapType == Map.m_MapType &&
		m_MapChannel == Map.m_MapChannel &&
		m_bTile[0] == Map.m_bTile[0] &&
		m_bTile[1] == Map.m_bTile[1] &&
		m_bTile[2] == Map.m_bTile[2] &&
		m_bMirror[0] == Map.m_bMirror[0] &&
		m_bMirror[1] == Map.m_bMirror[1] &&
		m_bMirror[2] == Map.m_bMirror[2] &&
		m_Transform == Map.m_Transform;
}

void CMaterialMap::PrepareRender( CRenderContext const& Context, LPDIRECT3DDEVICE9 pDevice, int32 SamplerIndex, float32 OffsetU, float32 OffsetV )
{
	if (pDevice == NULL)
	{
		return;
	}

	//Set texture
	LPDIRECT3DBASETEXTURE9 pTexture = NULL;
	if (m_hTexture != NULL)
	{
		pTexture = m_hTexture->GetD3DTexture();
	}
	Context.SetTexture( pDevice, SamplerIndex, pTexture );

	//Set texture transform
	if (m_bUseTransform || OffsetU || OffsetV)
	{
		assert( SamplerIndex < 8 );
		CMatrix4f Transform ( m_Transform );
		Transform.RowZ.x += OffsetU;
		Transform.RowZ.y += OffsetV;
		pDevice->SetTransform( static_cast<D3DTRANSFORMSTATETYPE>( D3DTS_TEXTURE0 + SamplerIndex ), 
			Transform.GetD3DMatrixPtr() );
		pDevice->SetTextureStageState( SamplerIndex, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
	} else {
		pDevice->SetTextureStageState( SamplerIndex, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	}

	uint32 TextureInput = D3DTSS_TCI_PASSTHRU;
	switch (m_MapType)
	{
		case EMaterialMap::KExplicit:
			break;
		case EMaterialMap::KSphere:
		case EMaterialMap::KCylinder: //Not implemented
			TextureInput = D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR;
			break;
		case EMaterialMap::KShrinkWrap:
			TextureInput = D3DTSS_TCI_CAMERASPACENORMAL;
			break;
		case EMaterialMap::KScreen:
			TextureInput = D3DTSS_TCI_CAMERASPACEPOSITION;
			break;
	}
	pDevice->SetTextureStageState( SamplerIndex, D3DTSS_TEXCOORDINDEX, TextureInput | m_MapChannel );

	/* Set the texture-addressing modes. Only the first two texture coordinates are relevant for 
	2D textures. Attempting to use D3DTADDRESS_BORDER for D3DSAMP_ADDRESSW results in D3D errors
	on some hardware; TODO check the relevant caps bits.*/
	for (uint32 i = 0; i < 2; ++i)
	{
		D3DSAMPLERSTATETYPE const State = static_cast<D3DSAMPLERSTATETYPE>( D3DSAMP_ADDRESSU + i );
		DWORD Address = D3DTADDRESS_WRAP;

		if (m_bTile[i])
		{
			if (m_bMirror[i])
			{
				Address = D3DTADDRESS_MIRROR;
			}
		} else {
			if (m_bMirror[i])
			{
				Address = D3DTADDRESS_MIRRORONCE;
			} else {
				Address = D3DTADDRESS_BORDER;
			}
		}

		pDevice->SetSamplerState( SamplerIndex, State, Address );
	}
}

int32 CMaterialMap::GetMapType() const
{
	return m_MapType;
}

void CMaterialMap::SetMapType( int32 MapType )
{
	if (MapType < 0 || MapType >= EMaterialMap::KCount)
	{
		throw std::invalid_argument( "material map type" );
		return;
	}

	m_MapType = static_cast<EMaterialMap::Type>( MapType );
}

int32 CMaterialMap::GetMapChannel() const
{
	return m_MapChannel;
}

void CMaterialMap::SetMapChannel( int32 MapChannel )
{
	if (MapChannel < 0)
	{
		return;
	}

	m_MapChannel = MapChannel;
}

bool CMaterialMap::GetUTile() const {return m_bTile[0];}
void CMaterialMap::SetUTile( bool bTile ) {m_bTile[0] = bTile;}
bool CMaterialMap::GetVTile() const {return m_bTile[1];}
void CMaterialMap::SetVTile( bool bTile ) {m_bTile[1] = bTile;}
bool CMaterialMap::GetWTile() const {return m_bTile[2];}
void CMaterialMap::SetWTile( bool bTile ) {m_bTile[2] = bTile;}
bool CMaterialMap::GetUMirror() const {return m_bMirror[0];}
void CMaterialMap::SetUMirror( bool bMirror ) {m_bMirror[0] = bMirror;}
bool CMaterialMap::GetVMirror() const {return m_bMirror[1];}
void CMaterialMap::SetVMirror( bool bMirror ) {m_bMirror[1] = bMirror;}
bool CMaterialMap::GetWMirror() const {return m_bMirror[2];}
void CMaterialMap::SetWMirror( bool bMirror ) {m_bMirror[2] = bMirror;}

CVector3f CMaterialMap::GetScale() const {return m_Scale;}

void CMaterialMap::SetScale( CVector3f const& Scale ) 
{
	m_Scale = Scale;
	UpdateTransform();
}

void CMaterialMap::SetScale( float32 ScaleX, float32 ScaleY, float32 ScaleZ )
{
	m_Scale.x = ScaleX;
	m_Scale.y = ScaleY;
	m_Scale.z = ScaleZ;
	UpdateTransform();
}

CVector3f const& CMaterialMap::GetRotation() const {return m_Rotation;}
void CMaterialMap::SetRotation( CVector3f const& Rotation )
{
	m_Rotation = Rotation;
	UpdateTransform();
}

void CMaterialMap::SetRotation( float32 Pitch, float32 Yaw, float32 Roll )
{
	m_Rotation.x = Pitch;
	m_Rotation.y = Yaw;
	m_Rotation.z = Roll;
	UpdateTransform();
}

CVector3f CMaterialMap::GetTranslation() const {return m_Translation;}

void CMaterialMap::SetTranslation( CVector3f const& Translation )
{
	m_Translation = Translation;
	UpdateTransform();
}

void CMaterialMap::SetTranslation( float32 TranslationX, float32 TranslationY, float32 TranslationZ )
{
	m_Translation.x = TranslationX;
	m_Translation.y = TranslationY;
	m_Translation.z = TranslationZ;
	UpdateTransform();
}

/*static*/ uint32 WTCall CMaterialMap::GetClassFormatVersion()
{
	return 2;
}

/*virtual*/ void CMaterialMap::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	Info->AddValue( "Name", m_Name );
	Info->AddValue( "TexturePath", m_TexturePath );
	Info->AddValue( "MapType", GetMapType() );	
	Info->AddValue( "MapChannel", m_MapChannel );
	Info->AddValue( "UTile", m_bTile[0] );
	Info->AddValue( "VTile", m_bTile[1] );
	Info->AddValue( "WTile", m_bTile[2] );
	Info->AddValue( "UMirror", m_bMirror[0] );
	Info->AddValue( "VMirror", m_bMirror[1] );
	Info->AddValue( "WMirror", m_bMirror[2] );
	Info->AddValue( "LuminanceAsAlpha", m_bUseLuminanceAsAlpha );
	Info->AddValue( "Scale", m_Scale );
	Info->AddValue( "RotationDegrees", m_Rotation * Math_RadToDeg( 1.0f ) );
	Info->AddValue( "Translation", m_Translation );
}

/*virtual*/ void CMaterialMap::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	Info->GetValue( "Name", m_Name );
	Info->GetValue( "TexturePath", m_TexturePath );
	int32 MapType;
	Info->GetValue( "MapType", MapType );	
	SetMapType( MapType );
	Info->GetValue( "MapChannel", m_MapChannel );
	Info->GetValue( "UTile", m_bTile[0] );
	Info->GetValue( "VTile", m_bTile[1] );
	Info->GetValue( "WTile", m_bTile[2] );
	Info->GetValue( "UMirror", m_bMirror[0] );
	Info->GetValue( "VMirror", m_bMirror[1] );
	Info->GetValue( "WMirror", m_bMirror[2] );

	if (Info->GetObjectFormatVersion() >= 2)
	{
		bool b; Info->GetValue( "LuminanceAsAlpha", b ); m_bUseLuminanceAsAlpha = b;
	} else {
		//Obsolete data
		m_bUseLuminanceAsAlpha = false;
	}

	Info->GetValue( "Scale", m_Scale );
	Info->GetValue( "RotationDegrees", m_Rotation );
	m_Rotation *= Math_DegToRad( 1.0f );
	Info->GetValue( "Translation", m_Translation );
}

}
}
