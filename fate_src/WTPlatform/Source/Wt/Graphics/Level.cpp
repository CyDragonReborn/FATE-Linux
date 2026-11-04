/***************************************************************************************
 *
 * Level.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 8/2/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"

#include "WT/System/Logging/Log.h"
#include "WT/Graphics/Level.h"
#include "WT/Graphics/StandardMaterial.h"

using namespace WT::Math;
using namespace WT::Graphics;
using namespace WT::System;
using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CLevel )

CLevel::CLevel()
	: m_pScene( NULL )
	, m_ResourcesValid( false )
	, m_FrustumCull( true )
	, m_SortOpaque( true )
	, m_SortTranslucent( true )
{
}

CLevel::CLevel( CScene* pScene )
	: m_pScene( pScene )
	, m_ResourcesValid( false )
	, m_FrustumCull( true )
	, m_SortOpaque( true )
	, m_SortTranslucent( true )
{
}

CLevel::~CLevel()
{
	//This is an awkward place to put this; but it needs to happen after
	//Materials and Material-related objects such as Models, Actors, and
	//CLevel's are loaded, but before the D3D device is closed.
	CTexture::FlushTextureCache();
}

void CLevel::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;

	for (CLevel::Models::iterator it = m_hModels.begin(); it != m_hModels.end(); ++it)
	{
		CModel* const pModel = *it;
		assert( pModel != NULL );
		pModel->InvalidateDeviceObjects();
	}
}

void CLevel::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	for (CLevel::Models::iterator it = m_hModels.begin(); it != m_hModels.end(); ++it)
	{
		CModel* const pModel = *it;
		assert( pModel != NULL );
		pModel->RestoreDeviceObjects( pDevice );
	}

	m_ResourcesValid = true;
}

bool CLevel::IsValid() const
{
	return m_ResourcesValid;
}

bool CLevel::GetFrustumCull() const {return m_FrustumCull;}
void CLevel::SetFrustumCull( bool FrustumCull ) {m_FrustumCull = FrustumCull;}
bool CLevel::GetSortOpaque() const {return m_SortOpaque;}
void CLevel::SetSortOpaque( bool SortOpaque ) {m_SortOpaque = SortOpaque;}
bool CLevel::GetSortTranslucent() const {return m_SortTranslucent;}
void CLevel::SetSortTranslucent( bool SortTranslucent ) {m_SortTranslucent = SortTranslucent;}

void CLevel::ClearModels()
{
	m_hModels.clear();
}

CModel& CLevel::GetModel( int32 Index )
{
	if (Index < 0 || Index >= static_cast<int32>( m_hModels.size() ))
	{
		throw std::out_of_range( "GetModel index" );
	}

	CModel* const pModel = m_hModels[Index];
	assert( pModel != NULL );
	return *pModel;
}

void CLevel::AddModel( CModel& Model )
{
	m_hModels.push_back( &Model );
}

int32 CLevel::FindModel( CModel const* pModel )
{
	Models::iterator it = std::find( m_hModels.begin(), m_hModels.end(), pModel );
	if (it == m_hModels.end())
	{
		return -1;
	}

	return static_cast<int32>( it - m_hModels.begin() );
}

void CLevel::RemoveModel( int32 Index )
{
	if (Index < 0 || Index >= static_cast<int32>( m_hModels.size() ))
	{
		throw std::out_of_range( "RemoveModel index" );
	}

	m_hModels.erase( m_hModels.begin() + Index );
}

/*virtual*/ void CLevel::Update( float32 ElapsedTimeSeconds )
{
	Models::iterator const End = m_hModels.end();

	for (Models::iterator it = m_hModels.begin(); it != End; ++it)
	{
		CModel* const pModel = *it;
		assert( pModel != NULL );
		pModel->Update( ElapsedTimeSeconds );
	}
}

//A structure containing a Model pointer and distances to that model
//from the camera. This is used to render objects from front-to-back
//(an optimization for opaque objects) or back-to-front (a necessity
//for translucent objects).

struct ModelInfo
{
	CModel*	m_pModel;
	float32	m_Nearest; //Nearest distance (used for front-to-back sorting)
	float32	m_Farthest; //Farthest distance (used for back-to-front sorting)

	ModelInfo( CModel* pModel, CMatrix4f const& ViewMatrix ) : m_pModel( pModel )
	{
		assert( m_pModel );

		m_Nearest = std::numeric_limits<float32>::max();
		m_Farthest = -std::numeric_limits<float32>::max();

		for (uint32 i = 0; i < EBoxVert::Count; ++i)
		{
			CAABox const& Box = m_pModel->GetBoundingBox();
			CVector3f Vert(
				i & (1 << 0) ? Box.Max.x : Box.Min.x,
				i & (1 << 1) ? Box.Max.y : Box.Min.y,
				i & (1 << 2) ? Box.Max.z : Box.Min.z
			);
		
			//Get world coordinate
			m_pModel->GetTransform().Transform( Vert, Vert );

			//Get view coordinate Z
			float32 const Z 
				= Vert.x * ViewMatrix.RowX.z
				+ Vert.y * ViewMatrix.RowY.z
				+ Vert.z * ViewMatrix.RowZ.z;
//				+ ViewMatrix.RowT.z; //Contribution unnecessary for sorting purposes

			if (m_Nearest > Z)
			{
				m_Nearest = Z;
			}

			if (m_Farthest < Z)
			{
				m_Farthest = Z;
			}
		}
	}
};

inline bool operator<( ModelInfo const& Model1, ModelInfo const& Model2 )
{
	return Model1.m_Nearest < Model2.m_Nearest;
};

inline bool operator>( ModelInfo const& Model1, ModelInfo const& Model2 )
{
	return Model1.m_Farthest > Model2.m_Farthest;
};

/*virtual*/ void CLevel::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return;
	}

	Models::iterator const End = m_hModels.end();
	bool const Cull = m_FrustumCull && Context.GetCamera() != NULL;

	if (Context.GetTranslucent() ? m_SortTranslucent : m_SortOpaque)
	{
		CMatrix4f ViewMatrix( false );
		pDevice->GetTransform( D3DTS_VIEW, ViewMatrix.GetD3DMatrixPtr() );

		std::vector<ModelInfo> ModelInfos;
		ModelInfos.reserve( m_hModels.size() );

		//Produce a list of frustum-culled objects
		for (Models::iterator it = m_hModels.begin(); it != End; ++it)
		{
			CModel* const pModel = *it;
			assert( pModel != NULL );

			if (Cull && !pModel->Intersects( Context.GetCamera()->GetFrustum() ))
			{
				++m_Statistics.m_CullFailed;
				continue;
			}

			++m_Statistics.m_CullPassed;
			ModelInfos.push_back( ModelInfo( pModel, ViewMatrix ) );
		}

		bool (*pPredicate)( ModelInfo const&, ModelInfo const& ) = 
			Context.GetTranslucent() ? 
			operator> /*translucent-only; sort back-to-front*/ : 
			operator< /*default; sort front-to-back*/;

		std::sort( ModelInfos.begin(), ModelInfos.end(), pPredicate );

		int32 ModelInfoCount = static_cast<int32>( ModelInfos.size() );
		for (int32 i = 0; i < ModelInfoCount; ++i)
		{
			ModelInfos[i].m_pModel->Render( pDevice, Context );
		}
	} else {
		//Produce a list of frustum-culled objects
		for (Models::iterator it = m_hModels.begin(); it != End; ++it)
		{
			CModel* const pModel = *it;
			assert( pModel != NULL );

			if (Cull && !pModel->Intersects( Context.GetCamera()->GetFrustum() ))
			{
				++m_Statistics.m_CullFailed;
				continue;
			}

			++m_Statistics.m_CullPassed;
			pModel->Render( pDevice, Context );
		}
	}
}

uint32 WTCall CLevel::GetClassFormatVersion() {return 3;}

/*virtual*/ void CLevel::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	GetClassType()->GetProperties()->SerializePropertyValues( this, Info, sContext );

	Info->AddContainerBeginWithCount( "Models", static_cast<uint32>( m_hModels.size() ) );

	for (Models::const_iterator it = m_hModels.begin(); it != m_hModels.end(); ++it)
	{
		CModel* const pModel = *it;
		assert( pModel != NULL );

		Info->AddValue( "item", pModel );
	}

	Info->AddContainerEnd();
}

/*virtual*/ void CLevel::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	//Initialize property values
	CRenderableImpl::InitPropertyValues();
	
	if (Info->GetObjectFormatVersion() >= 3)
	{
		GetClassType()->GetProperties()->DeserializePropertyValues( this, Info, sContext );
	} else if (Info->GetObjectFormatVersion() == 2) { //Obsolete version
		bool b;	Info->GetValue( "Visible", b ); SetVisible( b );
	}

	int32 ModelCount = Info->GetContainerBeginWithCount( "Models" );

	for (int32 i = 0; i < ModelCount; ++i)
	{
		CObject* pObject;
		Info->GetValue( "item", pObject );
		CModel* pModel = DynamicCast<CModel>( pObject );

		if (pModel == NULL)
		{
			m_hModels.clear();

			throw std::runtime_error( "corrupt stream" );
		}

		m_hModels.push_back( pModel );
	}

	Info->GetContainerEnd();
}

struct MirrorInfo
{
	LPDIRECT3DBASETEXTURE9 m_pDiffuseTexture;
	CMirror* m_pMirror;

	//The Model does not own its materials; neither shall the CLevel.
	//CLevel only guarantees that if it creates a mirror material,
	//that this material shall be used by at least one model.
	//The user's responsibility is to manage ownership for all model 
	//materials.
	CMaterial* m_pNewMaterial;
};

/*virtual*/ void CLevel::FinalizeDeserialization( CSerializationInfo* Info, CSerializationContext* sContext )
{
	//_BaseClass::FinalizeDeserialization( Info, sContext );

	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	if (pDevice == NULL)
	{
		throw std::invalid_argument( "NULL D3DDevice in serialization context" );
	}

	std::vector<MirrorInfo> MirrorInfos;

	if (m_pScene != NULL)
	{
		//Check for reflective materials assigned to geometry. If found,
		//ask the scene to create and manage reflection textures.

		for (CLevel::Models::iterator it = m_hModels.begin(); it != m_hModels.end(); ++it)
		{
			CModel* const pModel = *it;
			assert( pModel != NULL );

			//If the model has no mesh then any reflective materials will not even be visible
			CMesh* pMesh = pModel->GetMesh();
			if (pMesh == NULL)
			{
				continue;
			}

			std::vector<HMaterial>* pMaterials = pModel->GetMaterials().get();

			if (pMaterials == NULL)
			{
				continue;
			}

			for (int32 MaterialID = 0; MaterialID < static_cast<int32>( pMaterials->size() ); ++MaterialID)
			{
				CMaterial* pMaterial = pMaterials->operator[]( MaterialID );

				CStandardMaterial* pStandard = DynamicCast<CStandardMaterial>( pMaterial );

				if (pStandard == NULL || pStandard->GetMap( CStandardMaterial::KMapTypeReflection ) == NULL)
				{
					continue; //Not a reflective material
				}

				//Find a vertex that uses the material, and get the plane information from it.

				for (int32 i = 0; i < ERenderCategory::Count; ++i)
				{
					CMesh::Parts& parts = pMesh->GetParts( static_cast<ERenderCategory::Type>( i ) );
					for (int32 j = 0; j < static_cast<int32>( parts.size() ); ++j)
					{
						CMeshPart* pPart = parts[j];
						if (pPart == NULL)
						{
							continue;
						}

						//If the mesh part uses the reflective material, and if it has at least one
						//vertex, then we can choose a plane.
						if (pPart->GetMaterialID() == MaterialID && pPart->GetVertexCount())
						{
							int32 Vertex = pPart->GetVertex( 0 );

							//Get vertex position and normal information
							CVector3f Position = pMesh->GetVertexPosition( Vertex );
							CVector3f Normal = pMesh->GetVertexNormal( Vertex );

							//Transform reflective plane according to model's transformation.
							CPlane Plane( Normal, Position );
							Plane.Transform( pModel->GetTransform(), Plane );

							CMirror* pMirror = m_pScene->AddMirror( Plane );

							//Get the diffuse texture
							LPDIRECT3DBASETEXTURE9 pDiffuseTexture = NULL;
							if (pStandard != NULL)
							{
								CMaterialMap* pDiffuse = pStandard->GetDiffuseMap();
								if (pDiffuse != NULL && pDiffuse->GetTexture() != NULL)
								{
									pDiffuseTexture = pDiffuse->GetTexture()->GetD3DTexture();
								}
							}

							//Get or create the material used to show the reflection texture for this mirror
							//and diffuse texture.
							MirrorInfo* pInfo = NULL;
							for (int32 i = 0; i < static_cast<int32>( MirrorInfos.size() ); ++i)
							{
								if (MirrorInfos[i].m_pDiffuseTexture == pDiffuseTexture &&
									MirrorInfos[i].m_pMirror == pMirror)
								{
									pInfo = &MirrorInfos[i];
									break;
								}
							}

							if (pInfo == NULL)
							{
								MirrorInfo Info;
								Info.m_pDiffuseTexture = pDiffuseTexture;
								Info.m_pMirror = pMirror;

								//Create a new material used to combine the reflection and diffuse texture
								if (pDiffuseTexture == NULL)
								{
									Info.m_pNewMaterial = new CReflectionMaterial( *Info.m_pMirror );
								} else {
									Info.m_pNewMaterial = new CBlendedReflectionMaterial( *Info.m_pMirror, pDiffuseTexture );
								}

								MirrorInfos.push_back( Info );
								pInfo = &MirrorInfos.back();
							}

							int32 NewMaterialID = -1;

							//Find this material in the model's list
							for (int32 i = static_cast<int32>( pMaterials->size() ) - 1; i >= 0; --i)
							{
								if (pInfo->m_pNewMaterial == pMaterials->operator[]( i ))
								{
									NewMaterialID = i;
									break;
								}
							}

							//If the material is not already referenced by the model, then add
							//it to the model's material list.
							if (NewMaterialID == -1)
							{
								pMaterials->push_back( pInfo->m_pNewMaterial );
								NewMaterialID = static_cast<int32>( pMaterials->size() ) - 1;
							}

							pPart->SetMaterialID( NewMaterialID );
						}
					}
				}

			} //For each material
		} //For each model
	} //If there is a scene

	//TODO find and delete unused materials - some may be unused due to
	//being supplanted by mirror materials.

	if (m_pScene != NULL)
	{
		LOG_GREEN( "Info: CScene now has %d CMirror's.", m_pScene->GetMirrorCount() );
	}

	CTexture::FlushTextureCache();
}

} //namespace Graphics
} //namespace WT
