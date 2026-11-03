/***************************************************************************************
 *
 * Model.cpp
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

#include "WT/System/UserPropertyCollection.h"
#include "WT/Graphics/Model.h"
#include "WT/Image/Image.h"

using namespace WT::System;
using namespace WT::System::Serialization;
using namespace WT::Math;
using namespace WT::Graphics;
using namespace WT::Graphics::Animation;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CModel )

CModel::CModel() 
	: m_ResourcesValid( false )
{
	m_hMaterials = SmartPointer<std::vector<HMaterial> >( new std::vector<HMaterial>() );
}

CModel::CModel( CMesh* pMesh )
	: m_ResourcesValid( false )
	, m_hMesh( pMesh )
{
	m_hMaterials = SmartPointer<std::vector<HMaterial> >( new std::vector<HMaterial>() );
}

CModel::~CModel()
{
}

void CModel::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;

	if (m_hMaterials != NULL)
	{
		int32 MaterialCount = static_cast<int32>( m_hMaterials->size() );
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			m_hMaterials->operator[]( i )->InvalidateDeviceObjects();
		}
	}
}

void CModel::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	if (m_hMaterials != NULL)
	{
		int32 MaterialCount = static_cast<int32>( m_hMaterials->size() );
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			m_hMaterials->operator[]( i )->RestoreDeviceObjects( pDevice );
		}
	}

	m_ResourcesValid = true;
}

bool CModel::IsValid() const
{
	return m_ResourcesValid;
}

void CModel::SetMesh( CMesh* pMesh )
{
	m_hMesh = pMesh;
}

/*virtual*/ void CModel::Optimize()
{
	if (m_hMesh != NULL)
	{
		m_hMesh->Optimize();
	}
}

System::SmartPointer< std::vector<HMaterial> > const& CModel::GetMaterials() const
{
	return m_hMaterials;
}

void CModel::SetMaterials( System::SmartPointer<std::vector<HMaterial> > const& hMaterials )
{
	m_hMaterials = hMaterials;
}

CMaterial* CModel::FindMaterialByName( std::string const& Name ) const
{
	if (m_hMaterials == NULL)
	{
		return NULL;
	}

	int32 const MaterialCount = static_cast<int32>( m_hMaterials->size() );
	for (int32 i = 0; i < MaterialCount; ++i)
	{
		CMaterial *const pMaterial = m_hMaterials->operator[]( i );
		if (pMaterial->GetName() == Name)
		{
			return pMaterial;
		}
	}

	return NULL;
}

/*virtual*/ void CModel::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ) || m_hMesh == NULL)
	{
		return;
	}

	//If the mesh has no parts corresponding to the render category we are
	//rendering, then quit.
	//Note that to be object-oriented we should really have a function
	//like Mesh::NeedsRender( renderCategory ), we are using detailed
	//knowledge about how meshes work here.
	if (m_hMesh->GetParts( Context.GetRenderCategory() ).empty())
	{
		return;
	}

	//Use model's transform explicitly
	Context.SetWorldTransform( pDevice, GetTransform() );

	m_hMesh->Render( pDevice, Context, m_hMaterials.get() );
}

uint32 WTCall CModel::GetClassFormatVersion() {return 6;}

/*virtual*/ void CModel::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	GetClassType()->GetProperties()->SerializePropertyValues( this, Info, sContext);

	Info->AddValue( "Mesh", m_hMesh.get() );

	uint32 NumMaterials = 0;
	if (m_hMaterials != NULL)
	{
		 NumMaterials = static_cast<uint32>( m_hMaterials->size() );
	}

	Info->AddContainerBeginWithCount( "Materials", NumMaterials );

	for (uint32 i = 0; i < NumMaterials; ++i)
	{
		assert( m_hMaterials->operator[]( i ) != NULL );
		Info->AddValue<CObject*>( "item", m_hMaterials->operator[]( i ) );
	}

	Info->AddContainerEnd( );
}

bool CModel::Intersects( CFrustum const& frustum ) const
{
	uint32 TotalFlags = 0;

	for (int32 i = 0; i < EBoxVert::Count; ++i)
	{
		CVector3f Point(
			i & (1 << 0) ? m_BoundingBox.Max.x : m_BoundingBox.Min.x,
			i & (1 << 1) ? m_BoundingBox.Max.y : m_BoundingBox.Min.y,
			i & (1 << 2) ? m_BoundingBox.Max.z : m_BoundingBox.Min.z );
		
		//Transform bounding box vertex to world coordinates
		CVector3f TransformedPoint( false );
		GetTransform().TransformNoOverlap( TransformedPoint, Point );
		
		TotalFlags |= frustum.ComputeIntersectFlags( TransformedPoint );

		if (TotalFlags == EBoxIntersectFlags::All)
		{
			return true; //No plane is violated in every instance
		}
	}

	//At least one plane is violated consistently
	return false;
}

/*virtual*/ void CModel::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	if (Info->GetObjectFormatVersion() < 2)
	{
		throw std::runtime_error( "unsupported model file version" );
	}

	//Initialize property values
	CRenderableImpl::InitPropertyValues();
	SetTransform( CMatrix4f::gm_Identity );
	SetScaleComponent( Math::CVector3f::gm_UnitScale );
	SetRotationComponent( CQuaternion() );
	m_Name.clear();

	if (Info->GetObjectFormatVersion() >= 5)
	{
		GetClassType()->GetProperties()->DeserializePropertyValues( this, Info, sContext);
	} else {
		//Obsolete versions
		if (Info->GetObjectFormatVersion() >= 4)
		{
			Info->GetContainerBegin( "Movable" );
			
			CMatrix4f Transform( false );
			Info->GetValue( L"transform", Transform );
			SetTransform( Transform );

			CQuaternion RotationComponent( false );
			Info->GetValue( L"rotationComponent", RotationComponent );
			SetRotationComponent( RotationComponent );

			CVector3f ScaleComponent( false );
			Info->GetValue( L"scaleComponent", ScaleComponent );
			SetScaleComponent( ScaleComponent );

			Info->GetContainerEnd();
		} else {
			SetScaleComponent( 1.0f, 1.0f, 1.0f );
			SetRotationComponent( CQuaternion() );
			SetTransform( CMatrix4f::gm_Identity );
		}	
	}

	if (Info->GetObjectFormatVersion() >= 3 && Info->GetObjectFormatVersion() <= 5) //Obsolete versions
	{
		bool b; Info->GetValue( "Visible", b ); SetVisible( b );
	}

	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	if (pDevice == NULL)
	{
		throw std::invalid_argument( "NULL D3DDevice in serialization context" );
	}

	//Read the mesh
	CMesh* pMesh;
	Info->GetValue( "Mesh", pMesh );
	m_hMesh = HMesh( pMesh );

	//Get the root-level material property bags
	uint32 NumMaterials = Info->GetContainerBeginWithCount( "Materials" );

	if (NumMaterials)
	{
		m_hMaterials = SmartPointer<std::vector<HMaterial> >(
			new std::vector<HMaterial>( NumMaterials, NULL ) );
	} else {
		m_hMaterials = NULL;
	}

	for (uint32 i = 0; i < NumMaterials; ++i)
	{
		CMaterial* pMaterial;
		Info->GetValue( "item", pMaterial );
		m_hMaterials->operator[]( i ) = HMaterial( pMaterial );
	}

	Info->GetContainerEnd();
}

/*virtual*/ void CModel::FinalizeDeserialization(CSerializationInfo* Info, CSerializationContext* sContext)
{
	//_BaseClass::FinalizeDeserialization( Info, sContext );

	//For each mesh part which has a two-sided material, mark the mesh part as two-sided.
	//TODO as an optimization, meshes consisting wholly of two-sided parts could simply be rendered using
	//D3DCULL_NONE. This optimization also could apply to opaque or translucent part collections.
	for (ERenderCategory::Type RenderCategory = static_cast<ERenderCategory::Type>( 0 ); 
		RenderCategory < ERenderCategory::Count; 
		RenderCategory = static_cast<ERenderCategory::Type>( RenderCategory + 1 ))
	{
		CMesh::Parts const& Parts = GetMesh()->GetParts( RenderCategory );
		for (size_t j = 0; j < Parts.size(); ++j)
		{
			CMaterial const* pMaterial = GetMaterial( GetMaterials().get(), Parts[j]->GetMaterialID() );
			if (pMaterial != NULL && pMaterial->IsTwoSided())
			{
				Parts[j]->SetFlags( Parts[j]->GetFlags() | CMeshPart::EFlags::TwoSided );
			}
		}
	}

	//Compute a bounding box for the deserialized mesh
	ComputeBoundingBox();

	//If there is a mesh, then perform final post-deserialization work.
	if (m_hMesh != NULL)
	{
		m_hMesh->AcquireDeviceResources();
	}
}

void CModel::ComputeBoundingBox()
{
	m_BoundingBox.Reset();

	if (m_hMesh != NULL)
	{
		int32 VertexCount = m_hMesh->GetVertexCount();
		for (int32 i = 0; i < VertexCount; ++i)
		{
			m_BoundingBox.ExtendToEnclose( m_hMesh->GetVertexPosition( i ) );
		}
	}
}

} //namespace Graphics
} //namespace WT
