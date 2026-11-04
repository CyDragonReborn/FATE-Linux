/***************************************************************************************
 *
 * Mesh.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/6/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

// Wishlist:
// -Expose the serializable, user-defined per-vertex float properties in the Mesh interface.
// -Allow more configurable vertices - e.g. a mesh could have positions but no normals or colors,
//  or a number of texture coordinate components other than 2.
// -Add collidable flag and collision detection support. High-accuracy collisions must take morph
//	 targets and vertex blending into account, as well as texel transparency.
// -Add Optimize() method to optimize (e.g. remove duplicate verts & tri-strip) user-constructed meshes.
//  (Currently you could serialize & deserialize which is awkward.)
// -Add revolution/translation function - revolve around an axis, turning points into 
//	 lines and lines into tris.
// -Optimize rendering of alpha-blended geometry by sorting on mixture of range and material properties.
//  This may reduce render state changes.
// -Optimize sorting by biasing centroid Z to be a nonnegative value. Then, integer compares may be used.
// -Submit translucent mesh parts for overall scene depth sorting (not just within meshes).
// -Script module should be upgraded to expose methods that are declared with default parameters.
// -Support more than 4 matrix indices (hardware vertex blending cannot be used in this case).
// -Is there a way to fix anomalies due to non-transitive overlap of translucent line segments / triangles?
// -Implementation could exploit quadlists on platforms that support them.

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/stdafx.h"

#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include "WT/System/IO/MemoryStream.h"
#include "WT/Mesh/MeshPart.h"
#include "WT/Mesh/MeshFVA.h"
#include "WT/Mesh/MeshUA.h"
#include "WT/Mesh/MeshVtxProp.h"
#include "WT/Mesh/MeshVtxPropCol.h"
#include "WT/Mesh/Mesh.h"

#define USE_NVTRISTRIP
#ifdef USE_NVTRISTRIP
#include "WT/Mesh/NvTriStrip/NvTriStrip.h"
#else
#include "WT/Mesh/tri_stripper_101/tri_stripper.h"
#endif


#define SUPPORT_TRANSLUCENT_SORTING 0
//Don't sort translucent primitives at all - draw them in a fixed order.
//This usually produces visual errors.
//TODO we should probably have three sorting modes:
//1. No sorting
//2. Sorting within a single mesh
//3. Submission to global sorter.
//The default should be #3, which is the safest way to avoid visual errors.

using namespace WT::System::IO;
using namespace WT::System::Serialization;
using namespace WT::Math;
using namespace WT::Graphics;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS(CMesh)

// -------------------------------------------------------------------------------------
// Constants, Enums
// -------------------------------------------------------------------------------------

float32 const KSqrt2 = 1.4142135623730950488016887242097f;
float32 const KSqrt3 = 1.7320508075688772935274463415059f;
float32 const KSqrt5 = 2.2360679774997896964091736687313f;

ARGB const KWhite = 0xffffffffu;

namespace
{
	int32 const KTranslucentIBCount = 16; //Number of index buffers; must be a power of two
	uint32 const KTranslucentIndexBufferSize = 60; //Verts per index buffer
	LPDIRECT3DINDEXBUFFER9 ghTranslucentIB[KTranslucentIBCount] = {}; //Buffer for rendering sorted triangles
	uint32 gTranslucentIBRefCount = 0;

	//Create shared index buffers for translucent sorting if necessary.
	void AcquireTranslucentIndexBuffers( LPDIRECT3DDEVICE9 pDevice )
	{
#if	SUPPORT_TRANSLUCENT_SORTING
		if (gTranslucentIBRefCount > 0)
		{
			for (int32 i = 0; i < KTranslucentIBCount; ++i)
			{
				assert( ghTranslucentIB[i] != NULL );
			}
		}

		++gTranslucentIBRefCount;

		for (int32 i = 0; i < KTranslucentIBCount; ++i)
		{
			assert( ghTranslucentIB[i] == NULL );

			HRESULT hResult = pDevice->CreateIndexBuffer( KTranslucentIndexBufferSize * sizeof( int32 ),
				D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &ghTranslucentIB[i], NULL );
		
			if (FAILED( hResult ))
			{
				assert( ghTranslucentIB[i] == NULL );
				throw std::runtime_error( "index buffer creation failure" );
			} else {
				assert( ghTranslucentIB[i] != NULL );
			}
		}
#endif
	}

	void ReleaseTranslucentIndexBuffers()
	{
#if	SUPPORT_TRANSLUCENT_SORTING
		assert( gTranslucentIBRefCount != 0 );

		if (!--gTranslucentIBRefCount)
		{
			for (int32 i = 0; i < KTranslucentIBCount; ++i)
			{
				SafeRelease( ghTranslucentIB[i] );
			}
		}
#endif
	}

} //anonymous namespace

// Serialization-related constants and functions
namespace
{
	uint32 KMaxPropColCount = 255; //Maximum allowed number of property collections in a mesh
	int32 KMaxUserBitCount = 255; //Maximum allowed number of per-part user bits in a mesh

	//Get the default value for a particular property type
	float32 GetDefaultPropertyValue( int32 PropertyType )
	{
		float32 Result = 0.0f;

		switch (PropertyType)
		{
			case KVtxPropTypeColorR:
			case KVtxPropTypeColorG:
			case KVtxPropTypeColorB:
			case KVtxPropTypeColorA:
				Result = 255.0f;
				break;
		}

		return Result;
	}

} //anonymous namespace

// -------------------------------------------------------------------------------------
// Forward Declarations
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Implementations
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// Public Methods
// -------------------------------------------------------------------------------------

void CMesh::UpdateVertexFormat()
{
	m_VertexSize = sizeof( float32 ) * m_BetaCount + ((m_MatrixCount + 3) & ~3) +
		GetDiffuseBytes() + sizeof( Math::CVector2f ) * m_UVCount;

	m_VBVertexFormat = D3DFVF_XYZ;
	m_VBVertexSize = sizeof( CVector3f ); /*position*/
	
	if (m_bUseHardwareBlending)
	{
		assert( m_BetaCount >= 1 );
		assert( m_BetaCount <= 4 );
		assert( D3DFVF_XYZB1 == 4 + (1 << 1) );
		assert( D3DFVF_XYZB2 == 4 + (2 << 1) );
		assert( D3DFVF_XYZB3 == 4 + (3 << 1) );
		assert( D3DFVF_XYZB4 == 4 + (4 << 1) );
		assert( D3DFVF_XYZB5 == 4 + (5 << 1) );

		uint32 BCount = m_BetaCount + !!m_MatrixCount;
		m_VBVertexFormat = 4 + (BCount << 1);
		m_VBVertexSize += sizeof( float32 ) * BCount /*betas and matrix indices*/;

		if (m_MatrixCount)
		{
			m_VBVertexFormat |= D3DFVF_LASTBETA_UBYTE4;
		}
	}

	m_VBVertexFormat |= D3DFVF_NORMAL;
	m_VBVertexFormat |= m_UVCount << D3DFVF_TEXCOUNT_SHIFT;
	m_VBVertexSize += sizeof( CVector3f ) /*normal*/ + sizeof( CVector2f ) * m_UVCount /*UV's*/;

	if (m_HasDiffuse)
	{
		m_VBVertexFormat |= D3DFVF_DIFFUSE;
		m_VBVertexSize += sizeof( ARGB ) /*diffuse*/;
	}

	UpdateVertexDeclaration();
}

void CMesh::UpdateVertexDeclaration()
{
	if (m_pDevice == NULL)
	{
		SafeRelease( m_hVDecl );
		return;
	}

	std::vector<D3DVERTEXELEMENT9> Elements;
	Elements.reserve( 3 /*Position, normal, and end*/ + static_cast<int32>( m_HasDiffuse ) + m_UVCount );

	D3DVERTEXELEMENT9 Element;
	Element.Stream = 0;
	Element.Offset = 0;
	Element.Method = D3DDECLMETHOD_DEFAULT;
	Element.UsageIndex = 0;
	
	Element.Type = D3DDECLTYPE_FLOAT3;
	Element.Usage = D3DDECLUSAGE_POSITION;
	Elements.push_back( Element );
	Element.Offset = sizeof( CVector3f );

	Element.Type = D3DDECLTYPE_FLOAT3;
	Element.Usage = D3DDECLUSAGE_NORMAL;
	Elements.push_back( Element );
	Element.Offset += sizeof( CVector3f );

	if (m_HasDiffuse)
	{
		Element.Type = D3DDECLTYPE_D3DCOLOR;
		Element.Usage = D3DDECLUSAGE_COLOR;
		Elements.push_back( Element );
		Element.Offset += sizeof( ARGB );
	}

	for (int32 i = 0; i < m_UVCount; ++i)
	{
		Element.Type = D3DDECLTYPE_FLOAT2;
		Element.Usage = D3DDECLUSAGE_TEXCOORD;
		Element.UsageIndex = static_cast<BYTE>( i );
		Elements.push_back( Element );
		Element.Offset += sizeof( CVector2f );
	}

	D3DVERTEXELEMENT9 const End = D3DDECL_END();
	Elements.push_back( End );

	SafeRelease( m_hVDecl );
	if (FAILED( m_pDevice->CreateVertexDeclaration( &Elements[0], &m_hVDecl ) ))
	{
		throw std::runtime_error( "CreateVertexDeclaration failure" );
	}
	assert( m_hVDecl != NULL );
}

CMesh::CMesh( LPDIRECT3DDEVICE9 pDevice ) :
	m_pDevice( NULL ),
	m_hVB( NULL ),
	m_hVDecl( NULL ),
#if MESH_WHOLE_IB
	m_hIB( NULL ),
#endif
	m_UVCount( 0 ),
	m_BetaCount( 0 ),
	m_MatrixCount( 0 ),
	m_VertexCount( 0 ),
	m_UserBitCount( 0 ),
	m_VBVertexFormat( 0 ),
	m_VBVertexSize( 0 ),
	m_HasDiffuse( false ),
	m_bCanHardwareBlend( false ),
	m_bUseSoftwareBlending( false ),
	m_bUseHardwareBlending( false ),
	m_bSortTranslucent( true ),
	m_bDirtyVB( true ),
	m_bUsingTranslucentIB( false ),
	m_Static( true ),
	m_pRenderGeoData( NULL )
{
	SetDevice( pDevice );

	UpdateVertexFormat();
}

CMesh::CMesh( CMesh const& mesh ) :
	m_pDevice( NULL ),
	m_hVB( NULL ),
	m_hVDecl( mesh.m_hVDecl ),
#if MESH_WHOLE_IB
	m_hIB( NULL ),
#endif
	m_UVCount( mesh.m_UVCount ),
	m_BetaCount( mesh.m_BetaCount ),
	m_MatrixCount( mesh.m_MatrixCount ),
	m_VertexSize( mesh.m_VertexSize ),
	m_VertexCount( mesh.m_VertexCount ),
	m_UserBitCount( mesh.m_UserBitCount ),
	m_VertexData( mesh.m_VertexData ),
	m_GeoData( mesh.m_GeoData ),
	m_VBVertexFormat( mesh.m_VBVertexFormat ),
	m_VBVertexSize( mesh.m_VBVertexSize ),
	m_HasDiffuse( mesh.m_HasDiffuse ),
	m_bCanHardwareBlend( mesh.m_bCanHardwareBlend ),
	m_bUseSoftwareBlending( mesh.m_bUseSoftwareBlending ),
	m_bUseHardwareBlending( mesh.m_bUseHardwareBlending ),
	m_bSortTranslucent( mesh.m_bSortTranslucent ),
	m_bDirtyVB( true ),
	m_bUsingTranslucentIB( false ),
	m_Static( mesh.m_Static ),
	m_pRenderGeoData( NULL )
{
	assert( m_hVDecl != NULL );
	m_hVDecl->AddRef();

	SetDevice( mesh.m_pDevice );

	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < mesh.m_hParts[i].size(); ++j)
		{
			CMeshPart const* pSrcPart = mesh.m_hParts[i][j];

			HMeshPart hPart = pSrcPart->Clone();
			hPart->SetMesh( this );

			m_hParts[i].push_back( hPart );
		}
	}
}

CMesh::~CMesh()	
{
	Clear();
	SafeRelease( m_hVDecl );
	SafeRelease( m_pDevice );
}

void CMesh::InvalidateDeviceObjects()
{
	//Release resources that are in the default pool
	if (!m_Static)
	{
		SafeRelease( m_hVB );
	}
}

void CMesh::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	assert( m_pDevice == pDevice );
	AcquireDeviceResources();
}

bool CMesh::IsValid() const
{
	return !m_VertexCount || m_hVB != NULL;
}

HMesh CMesh::Clone() const
{
	return new CMesh( *this );
}

void CMesh::SetDeviceAux( LPDIRECT3DDEVICE9 pDevice )
{
	assert( m_pDevice != pDevice );

	if (pDevice != NULL)
	{
		pDevice->AddRef();
	}
	SafeRelease( m_pDevice );
	m_pDevice = pDevice;

	ReleaseDeviceResources();

	//If the vertex declaration is NULL, if the new device is NULL, or if there is a valid vertex declaration 
	//but it uses a different device, then the vertex declaration should be discarded and/or replaced. The
	//existing vertex declaration will remain valid in some cases, e.g. when using the copy constructor.
	bool UpdateVertexDecl = false;
	if (m_hVDecl == NULL || pDevice == NULL)
	{
		UpdateVertexDecl = true;
	} else {
		LPDIRECT3DDEVICE9 pVertexDeclDevice;
		HRESULT hResult = m_hVDecl->GetDevice( &pVertexDeclDevice );
		if (FAILED( hResult ))
		{
			WT_THROW( System::CRuntimeException, "GetDevice failure" );
		}

		assert( pVertexDeclDevice != NULL );
		if (pVertexDeclDevice != pDevice)
		{
			UpdateVertexDecl = true;
		}
		pVertexDeclDevice->Release();
	}

	if (UpdateVertexDecl)
	{
		UpdateVertexDeclaration();
	}

	bool bCanHardwareBlend = false;
	if (pDevice != NULL)
	{
		D3DCAPS9 Caps;
		HRESULT Result = pDevice->GetDeviceCaps( &Caps );
		
		if (SUCCEEDED( Result ) &&
			Caps.MaxVertexBlendMatrices >= KMaxMatrixCount &&
			Caps.MaxVertexBlendMatrixIndex > 0)
		{
//			bCanHardwareBlend = true;
			//Leave hardware skinning disabled for now.
			//-TODO fix the bug on ATI cards.
			//-TODO find a way for hardware skinning to be compatible with vertex shaders.
		}
	}

	if (m_bCanHardwareBlend != bCanHardwareBlend)
	{
		m_bCanHardwareBlend = bCanHardwareBlend;
		UpdateBlendingMode();
	}
}

void CMesh::ReleaseDeviceResources()
{
	//Can't use the old buffer if device new / changed
	//because we need to create a VB using the device
	//and also the VB format will vary based on whether the device
	//supports indexed blending.
	SafeRelease( m_hVB );
#if MESH_WHOLE_IB
	SafeRelease( m_hIB );
#endif

	if (m_bUsingTranslucentIB)
	{
		ReleaseTranslucentIndexBuffers();
		m_bUsingTranslucentIB = false;
	}

	m_bCanHardwareBlend = false;
}

void CMesh::Clear()
{
	ReleaseDeviceResources();

	m_BetaCount = 0;
	m_MatrixCount = 0;
	m_HasDiffuse = false;
	m_UVCount = 0;

	UpdateVertexFormat();

	ClearVertices();
	m_UserBitCount = 0;

	UpdateBlendingMode();
}

//Translucent rendering aids
namespace
{
	void EnableAlphaBlendState( LPDIRECT3DDEVICE9 pDevice , bool TranslucentZWrite = false )
	{
		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TranslucentZWrite ? TRUE : FALSE );
	}

	void DisableAlphaBlendState( LPDIRECT3DDEVICE9 pDevice )
	{
		//Turn off alpha blending
		pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}

	struct PrimInfo
	{
		void Init( int32 VertexCount, CMaterial const* pMaterial, bool bTwoSided )
		{
			m_VertexCount = VertexCount;
			m_pMaterial = pMaterial;
			m_bTwoSided = bTwoSided;
		}

		float32		 m_Z;				//Computed Z value of centroid - used for sorting
		int32		 m_VertexCount;		//Number of vertices - 1 (point), 2 (line), or 3 (tri)
		int32		 m_Vertices[3];		//Vertex indices
		CMaterial const*	m_pMaterial;		//Material
		bool		 m_bTwoSided : 1;	//Two-sided primitive?
	};

	//Primitive comparison functor
	struct PrimGreater : std::greater< PrimInfo* >
	{
		result_type operator()( first_argument_type arg1, second_argument_type arg2 )
		{
			return arg1->m_Z > arg2->m_Z;
		}
	};
} //anonymous namespace

void CMesh::RenderTranslucentWithSorting( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context,
										  Parts const& parts,
										  std::vector<HMaterial> const* pMaterials ) 
{
#if	SUPPORT_TRANSLUCENT_SORTING
	assert( !parts.empty() ); //Should have been checked in Render()

	//Initialize translucent index buffers
	if (!m_bUsingTranslucentIB)
	{
		AcquireTranslucentIndexBuffers( pDevice );
		m_bUsingTranslucentIB = true;
	}

	CMatrix4f ToViewMatrix( false );
	pDevice->GetTransform( D3DTS_VIEW, ToViewMatrix.GetD3DMatrixPtr() );

	//If hardware blending is not being used, then the world matrix affects 
	//centroid positions in view space.
	bool const bHardwareBlending = (m_MatrixCount | m_BetaCount) && m_bCanHardwareBlend;
	if (!bHardwareBlending)
	{
		CMatrix4f WorldMatrix( false );
		pDevice->GetTransform( D3DTS_WORLD, WorldMatrix.GetD3DMatrixPtr() );
		ToViewMatrix = WorldMatrix * ToViewMatrix;
	}

	//Compute the total number of prims to sort
	int32 PrimCount = 0;
	size_t const TranslucentPartCount = parts.size();
	for (size_t i = 0; i < TranslucentPartCount; ++i)
	{
		CMeshPart* pPart = parts[i];

		if (pPart->GetFlags() & CMeshPart::EFlags::Invisible)
		{
			continue;
		}

		int32 IndexCount = pPart->GetVertexCount();
		
		switch (pPart->GetPartType())
		{
		default:
			assert( false );
			continue;
		case CMeshPart::EType::PointList:
			PrimCount += IndexCount;
			break;
		case CMeshPart::EType::LineList:
			PrimCount += IndexCount >> 1;
			break;
		case CMeshPart::EType::LineStrip:
			if (IndexCount >= 2) PrimCount += IndexCount - 1;
			break;
		case CMeshPart::EType::TriList:
			PrimCount += IndexCount / 3;
			break;
		case CMeshPart::EType::TriStrip:
		case CMeshPart::EType::TriStripInverted:
		case CMeshPart::EType::TriFan:
			if (IndexCount >= 3) PrimCount += IndexCount - 2;
			break;
		case CMeshPart::EType::QuadList:
			PrimCount += (IndexCount >> 2) << 1;
			break;
		}
	}

	if (PrimCount == 0)
	{
		return;
	}

	//List of pairs of mesh part index and primitive (point, line, or triangle) index
	std::vector< PrimInfo > Prims( PrimCount );

	PrimInfo* pPrimOut = &Prims[0];
	
	for (size_t i = 0; i < TranslucentPartCount; ++i)
	{
		CMeshPart* pPart = parts[i];

		if (pPart->GetFlags() & CMeshPart::EFlags::Invisible)
		{
			continue;
		}

		int32 IndexCount = pPart->GetVertexCount();
		CMaterial const* pMaterial = GetMaterial( pMaterials, pPart->GetMaterialID() );
		bool bTwoSided = pPart->GetFlags() & CMeshPart::EFlags::TwoSided;
		
		switch (pPart->GetPartType())
		{
		default:
			assert( false );
			continue;
		case CMeshPart::EType::PointList:
			for (int32 i = 0; i < IndexCount; ++i, ++pPrimOut)
			{
				pPrimOut->Init( 1, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( i );
			}
			break;
		case CMeshPart::EType::LineList:
			for (int32 i = 1; i < IndexCount; i += 2, ++pPrimOut)
			{
				pPrimOut->Init( 2, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( i - 1 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( i );
			}
			break;
		case CMeshPart::EType::LineStrip:
			for (int32 i = 1; i < IndexCount; ++i, ++pPrimOut)
			{
				pPrimOut->Init( 2, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( i - 1 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( i );
			}
			break;
		case CMeshPart::EType::TriList:
			for (int32 i = 2; i < IndexCount; i += 3, ++pPrimOut)
			{
				pPrimOut->Init( 3, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( i - 2 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( i - 1 );
				pPrimOut->m_Vertices[2] = pPart->GetVertex( i );
			}
			break;
		case CMeshPart::EType::TriStrip:
			for (int32 i = 2; i < IndexCount; ++i, ++pPrimOut)
			{
				pPrimOut->Init( 3, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( (i - 1) & ~1 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( (i - 2) | 1 );
				pPrimOut->m_Vertices[2] = pPart->GetVertex( i );
			}
			break;
		case CMeshPart::EType::TriFan:
			for (int32 i = 2; i < IndexCount; ++i, ++pPrimOut)
			{
				pPrimOut->Init( 3, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( 0 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( i - 1 );
				pPrimOut->m_Vertices[2] = pPart->GetVertex( i );
			}
			break;
		case CMeshPart::EType::TriStripInverted:
			for (int32 i = 2; i < IndexCount; ++i, ++pPrimOut)
			{
				pPrimOut->Init( 3, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( (i - 2) | 1 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( (i - 1) & ~1 );
				pPrimOut->m_Vertices[2] = pPart->GetVertex( i );
			}
		case CMeshPart::EType::QuadList:
			for (int32 i = 3; i < IndexCount; i += 4, ++pPrimOut)
			{
				pPrimOut->Init( 3, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( i - 3 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( i - 2 );
				pPrimOut->m_Vertices[2] = pPart->GetVertex( i - 1 );
				++pPrimOut;
				pPrimOut->Init( 3, pMaterial, bTwoSided );
				pPrimOut->m_Vertices[0] = pPart->GetVertex( i - 1 );
				pPrimOut->m_Vertices[1] = pPart->GetVertex( i - 0 );
				pPrimOut->m_Vertices[2] = pPart->GetVertex( i - 3 );
			}
			break;
		}
	}
	assert( pPrimOut == &Prims[PrimCount] );

	//Compute the factors used to compute centroid locations. ToViewMatrix._43 can be neglected as 
	//it merely biases all Z values equally (note - this would not be true if considering multiple 
	//meshes with differing world matrices).

	static float32 const fReciprocals[3] = {1.0f, 0.5f, 1.0f / 3.0f,};
	float32 Factors[3][4];
	for (int32 i = 0; i < 3; ++i)
	{
		Factors[i][0] = ToViewMatrix._13 * fReciprocals[i];
		Factors[i][1] = ToViewMatrix._23 * fReciprocals[i];
		Factors[i][2] = ToViewMatrix._33 * fReciprocals[i];
		Factors[i][3] = 0.0f;
	}

	GeometryData* pGeoData = m_pRenderGeoData;
	if (pGeoData == NULL)
	{
		pGeoData = &m_GeoData[0];
	}

	//Compute the Z-value of the centroid of the primitive (point / line / tri).
	//This is the average of the Z-values of each vertex.
	for (int32 p = 0; p < PrimCount; ++p)
	{
		PrimInfo& Info = Prims[p];

		CVector3f Centroid = pGeoData[Info.m_Vertices[0]].m_Position;
		for (int32 i = 1; i < Info.m_VertexCount; ++i)
		{
			CVector3f const& Position = pGeoData[Info.m_Vertices[i]].m_Position;
			Centroid += Position;
		}

		float32 const *const fFactors = &Factors[ Info.m_VertexCount - 1 ][0];
		Info.m_Z = Centroid.x * fFactors[0] + Centroid.y * fFactors[1] + Centroid.z * fFactors[2];
	}

	//Sort in descending order (farthest to nearest).
	std::vector< PrimInfo* > PrimPtrs( PrimCount );
	for (int32 p = 0; p < PrimCount; ++p)
	{
		PrimPtrs[p] = &Prims[p];
	}
	std::sort( &PrimPtrs[0], &PrimPtrs[PrimCount], PrimGreater() ); 

	int32 CurTranslucentIndex = 0;
	int32 CurDrawPrimCount = 0;
	D3DPRIMITIVETYPE CurPrimitiveType = D3DPT_TRIANGLELIST;
	int32 MinVertexIndex = std::numeric_limits<int32>::max();
	int32 MaxVertexIndex = 0;
	int32 IndexCount;
	int32 CurIB = 0;
	int32* pIndices;
	ghTranslucentIB[CurIB]->Lock( 0, sizeof( int32 ) * KTranslucentIndexBufferSize, reinterpret_cast<void**>( &pIndices ),
		0 );

	//Render each prim
	for (int32 p = 0; p < PrimCount; ++p)
	{
		PrimInfo const& Info = *PrimPtrs[p];

		//Points are not rendered with index buffers, so no index buffer
		//management is required.
		if (Info.m_VertexCount == 1)
		{
			pDevice->DrawPrimitive( D3DPT_POINTLIST, Info.m_Vertices[0], 1 );
			continue;
		}

		D3DPRIMITIVETYPE DesiredPrimitiveType =
			Info.m_VertexCount == 2 ? D3DPT_LINELIST : D3DPT_TRIANGLELIST;

		IndexCount = Info.m_VertexCount;
		//It means nothing for a Line segments to be two sided.
		bool bTwoSided = Info.m_bTwoSided && (Info.m_VertexCount != 2);
		if (bTwoSided)
		{
			IndexCount <<= 1;
		}

		if (CMaterial::GetCachedMaterial() != Info.m_pMaterial ||
			CurPrimitiveType != DesiredPrimitiveType ||
			CurTranslucentIndex + IndexCount > KTranslucentIndexBufferSize)
		{
			//Render the current primitives
			ghTranslucentIB[CurIB]->Unlock();

			if (CurTranslucentIndex)
			{
				pDevice->SetIndices( ghTranslucentIB[CurIB] );
				pDevice->DrawIndexedPrimitive(
					CurPrimitiveType, 0,
					//Pass range of used vertices - hopefully an optimization
					MinVertexIndex, MaxVertexIndex - MinVertexIndex + 1,
					0,
					CurDrawPrimCount
				);

				CurTranslucentIndex = 0;
				CurDrawPrimCount = 0;
			} else {
				assert( CurDrawPrimCount == 0 );
			}

			CurPrimitiveType = DesiredPrimitiveType;
			MinVertexIndex = std::numeric_limits<int32>::max();
			MaxVertexIndex = 0;

			//Increment current index buffer, wrapping back to zero if KTranslucentIBCount is reached.
			//Make sure that we can use operator& to do the wrapping - this amounts to requiring
			//that KTranslucentIBCount is a power of two.
			assert( ((CurIB + 1) & (KTranslucentIBCount - 1)) == (CurIB + 1) % KTranslucentIBCount );
			CurIB = (CurIB + 1) & (KTranslucentIBCount - 1);
			ghTranslucentIB[CurIB]->Lock( 0, sizeof( int32 ) * KTranslucentIndexBufferSize, reinterpret_cast<void**>( &pIndices ),
				0 );
		}

		//Check if the current material needs to change
		Material::SetCachedMaterial( pDevice, Context, Info.m_pMaterial );

		for (int32 i = 0; i < Info.m_VertexCount; ++i)
		{
			pIndices[CurTranslucentIndex + i] = Info.m_Vertices[i];
		}
		++CurDrawPrimCount;

		if (bTwoSided)
		{
			for (int32 i = 0; i < Info.m_VertexCount; ++i)
			{
				pIndices[CurTranslucentIndex + Info.m_VertexCount + i] = Info.m_Vertices[Info.m_VertexCount - 1 - i];
			}
			++CurDrawPrimCount;
		}

		for (int32 i = 0; i < IndexCount; ++i)
		{
			if (MinVertexIndex > pIndices[CurTranslucentIndex + i])
			{
				MinVertexIndex = pIndices[CurTranslucentIndex + i];
			}
			
			if (MaxVertexIndex < pIndices[CurTranslucentIndex + i]) {
				MaxVertexIndex = pIndices[CurTranslucentIndex + i];
			}
		}

		CurTranslucentIndex += IndexCount;
	}

	//Render the last remaining primitives
	ghTranslucentIB[CurIB]->Unlock();

	if (CurTranslucentIndex)
	{
		pDevice->SetIndices( ghTranslucentIB[CurIB] );
		pDevice->DrawIndexedPrimitive(
			CurPrimitiveType, 0,
			//Pass range of used vertices - hopefully an optimization
			MinVertexIndex, MaxVertexIndex - MinVertexIndex + 1,
			0,
			CurDrawPrimCount
		);
	}
#endif
}

void CMesh::AcquireDeviceResourcesMesh()
{
	//Create vertex buffer
	if (m_pDevice == NULL)
	{
		throw std::runtime_error( "NULL device" );
	}

	assert( m_hVDecl != NULL );

	if (!m_VertexCount)
	{
		assert( m_hVB == NULL );
		return;
	}

#if 0
	//TODO - sometimes a static mesh which changes to frequently or a dynamic mesh which hasn't
	//changed in a long time. Based on a heuristic allow a static mesh to become dynamic or 
	//vice versa.
	{
		//Switch from static to dynamic or vice versa
		SafeRelease( m_hVB );
		m_Static = !m_Static;
	}
#endif

	UpdateBlendingMode();

	if (m_hVB == NULL)
	{
		assert( m_hVDecl != NULL ); //Vertex declaration as well as format should be up-to-date

		//Create a new vertex buffer
		m_pDevice->CreateVertexBuffer( m_VertexCount * m_VBVertexSize,
			m_Static ? D3DUSAGE_WRITEONLY : D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, m_VBVertexFormat, 
			m_Static ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT,
			 &m_hVB, NULL );

		if (m_hVB == NULL)
		{
			throw std::runtime_error( "CreateVertexBuffer failure" );
		}

		m_bDirtyVB = true;
	}

	if (m_bDirtyVB)
	{
		uint8* pVertex;
		HRESULT Result = m_hVB->Lock( 0, m_VertexCount * m_VBVertexSize, 
			reinterpret_cast<void**>( &pVertex ), m_Static ? 0 : D3DLOCK_DISCARD );

		if (FAILED( Result ))
		{
			throw std::runtime_error( "Lock vertex buffer failure" );
		}

		FillVB( pVertex );
		FillVBGeometry( pVertex );
		m_bDirtyVB = false;

		m_hVB->Unlock();
	}

#if MESH_WHOLE_IB
	if (m_hIB == NULL)
	{
		int32 StartIndex = 0;
		for (int32 i = 0; i < ERenderCategory::Count; ++i)
		{
			Parts& parts = m_hParts[i];
			int32 PartCount = static_cast<int32>( parts.size() );
			for (int32 j = 0; j < PartCount; ++j)
			{
				CMeshPart* pPart = parts[j];

				pPart->m_StartIndex = StartIndex;
				StartIndex += pPart->GetIndexCount();
			}
		}

		m_pDevice->CreateIndexBuffer( StartIndex * sizeof( int16 ),
			D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_hIB, NULL );
		if (m_hIB == NULL)
		{
			throw std::runtime_error( "CreateIndexBuffer failure" );
		}

		void* pData;
		HRESULT Result = m_hIB->Lock( 0, StartIndex * sizeof( int16 ), &pData, 0 );
		if (FAILED( Result ))
		{
			throw std::runtime_error( "index buffer lock" );
		}

		for (int32 i = 0; i < ERenderCategory::Count; ++i)
		{
			Parts& parts = m_hParts[i];
			int32 PartCount = static_cast<int32>( parts.size() );
			for (int32 j = 0; j < PartCount; ++j)
			{
				CMeshPart* pPart = parts[j];

				pPart->FillIndexBuffer( static_cast<uint8*>( pData ) + 
					(pPart->m_StartIndex * sizeof(int16)), sizeof(int16) );
			}
		}

		m_hIB->Unlock();
	}
#endif
}

//Internal update function - updates m_bUseSoftwareBlending and m_bUseHardwareBlending flags.
//Actual software blending is not the responsibility of Mesh...
void CMesh::UpdateBlendingMode()
{
	m_bUseSoftwareBlending = (m_MatrixCount | m_BetaCount) && !m_bCanHardwareBlend;
	m_bUseHardwareBlending = (m_MatrixCount | m_BetaCount) && m_bCanHardwareBlend;
}

void CMesh::AcquireDeviceResources()
{
	AcquireDeviceResourcesMesh();

	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			m_hParts[i][j]->AcquireDeviceResources();
		}
	}
}

void CMesh::Transform( CMatrix4f const& Transform )
{
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		CVector3f Result( false );
		
		Transform.TransformNoOverlap( Result, GetVertexPosition( i ) );
		SetVertexPosition( i, Result );

		Transform.RotateNoOverlap( Result, GetVertexNormal( i ) );
		Result.Normalize();
		SetVertexNormal( i, Result );
	}

	//The determinant of the transformation tells us whether the mesh parts
	//need their orientations inverted. If the determinant is negative,
	//then the transformation consists of an odd number of reflections and
	//mesh parts must be flipped.
	if (Transform.ComputeDeterminant3x3() < 0.0f)
	{
		InvertParts();
	}
}

void CMesh::Scale( float32 Scale )
{
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		SetVertexPosition( i, GetVertexPosition( i ) * Scale );
	}

	if (Scale < 0.0f)
	{
		for (int32 i = 0; i < m_VertexCount; ++i)
		{
			SetVertexNormal( i, -GetVertexNormal( i ) );
		}
		InvertParts();
	}
}

void CMesh::Scale( CVector3f const& Scale )
{
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		CVector3f Position = GetVertexPosition( i );
		SetVertexPosition( i, Position.x * Scale.x, Position.y * Scale.y, Position.z * Scale.z );
		
		CVector3f Normal = GetVertexNormal( i );
		Normal.x *= Scale.x;
		Normal.y *= Scale.y;
		Normal.z *= Scale.z;
		Normal.Normalize();
		SetVertexNormal( i, Normal );
	}

	if (Scale.x * Scale.y * Scale.z < 0.0f)
	{
		InvertParts();
	}
}

void CMesh::Scale( float32 ScaleX, float32 ScaleY, float32 ScaleZ )
{
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		CVector3f Position = GetVertexPosition( i );
		SetVertexPosition( i, Position.x * ScaleX, Position.y * ScaleY, Position.z * ScaleZ );
		
		CVector3f Normal = GetVertexNormal( i );
		Normal.x *= ScaleX;
		Normal.y *= ScaleY;
		Normal.z *= ScaleZ;
		Normal.Normalize();
		SetVertexNormal( i, Normal );
	}

	if (ScaleX * ScaleY * ScaleZ < 0.0f)
	{
		InvertParts();
	}
}

void CMesh::Translate( CVector3f const& Translation )
{
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		SetVertexPosition( i, GetVertexPosition( i ) + Translation );
	}
}

void CMesh::Translate( float32 TranslateX, float32 TranslateY, float32 TranslateZ )
{
	CVector3f Translate( TranslateX, TranslateY, TranslateZ );
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		SetVertexPosition( i, GetVertexPosition( i ) + Translate );
	}
}

int32 CMesh::GetVertexCount() const
{
	return m_VertexCount;
}

int32 CMesh::GetUserBitCount() const
{
	return m_UserBitCount;
}

int32 CMesh::ComputeRenderablePrimCount() const
{
	int32 Result = 0;

	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart const* pPart = m_hParts[i][j];

			if (pPart->GetFlags() & CMeshPart::EFlags::Invisible)
			{
				continue;
			}

			Result += pPart->GetPrimCount();
		}
	}

	return Result;
}

int32 CMesh::GetUVCount() const
{
	return m_UVCount;
}

int32 CMesh::GetBetaCount() const
{
	return m_BetaCount;
}

int32 CMesh::GetMatrixCount() const
{
	return m_MatrixCount;
}

CVector3f CMesh::GetVertexPosition( int32 Index ) const
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "GetVertexPosition index" );
	}

    return m_GeoData[Index].m_Position;
}

CVector3f CMesh::GetVertexNormal( int32 Index ) const
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "GetVertexNormal index" );
	}

    return m_GeoData[Index].m_Normal;
}

ARGB CMesh::GetVertexDiffuse( int32 Index ) const
{
	if (!m_HasDiffuse)
	{
		throw std::runtime_error( "GetVertexDiffuse on CMesh that lacks diffuse colors" ); 
	}

	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "GetVertexDiffuse index" );
	}

	uint32 const* pDiffuse = reinterpret_cast<uint32 const*>( GetVertexPtr( Index ) + GetBetaMatrixBytes() );
	return ARGB( *pDiffuse );
}

CVector2f CMesh::GetVertexUV( int32 Index, int32 StageIndex ) const
{
	if (Index < 0 || Index >= m_VertexCount || StageIndex < 0 || StageIndex >= m_UVCount)
	{
		throw std::out_of_range( "GetVertexUV indices" );
	}

	CVector2f const* pUV = reinterpret_cast<CVector2f const*>( GetVertexPtr( Index ) 
		+ GetUVByteOffset() );

	return pUV[StageIndex];
}

float32 CMesh::GetVertexBeta( int32 Index, int32 BlendIndex ) const
{
	if (Index < 0 || Index >= m_VertexCount || BlendIndex < 0 || BlendIndex >= m_BetaCount)
	{
		throw std::out_of_range( "GetVertexBeta indices" );
	}

	float32 const* pBeta = reinterpret_cast<float32 const*>( GetVertexPtr( Index ) );
	return pBeta[BlendIndex];
}

int32 CMesh::GetVertexMatrix( int32 Index, int32 MatrixIndex ) const
{
	if (Index < 0 || Index >= m_VertexCount || MatrixIndex < 0 || MatrixIndex >= m_MatrixCount)
	{
		throw std::out_of_range( "GetVertexMatrix indices" );
	}

	uint8 const* pMatrices = GetVertexPtr( Index ) + GetBetaBytes();
	return pMatrices[MatrixIndex];
}

CMesh::Parts& CMesh::GetParts( ERenderCategory::Type RenderCategory )
{
	if (RenderCategory < 0 || RenderCategory >= ERenderCategory::Count)
	{
		throw std::out_of_range( "mesh part render category" );
	}

	return m_hParts[RenderCategory];
}

CMesh::Parts const& CMesh::GetParts( ERenderCategory::Type RenderCategory ) const
{
	if (RenderCategory < 0 || RenderCategory >= ERenderCategory::Count)
	{
		throw std::out_of_range( "mesh part render category" );
	}

	return m_hParts[RenderCategory];
}

bool CMesh::GetSortTranslucent() const
{
	return m_bSortTranslucent;	
}

void CMesh::SetSortTranslucent( bool bSortTranslucent )
{
	if (!bSortTranslucent && m_bSortTranslucent)
	{
		if (m_bUsingTranslucentIB)
		{
			ReleaseTranslucentIndexBuffers();
			m_bUsingTranslucentIB = false;
		}
	}

	m_bSortTranslucent = bSortTranslucent;
}

bool CMesh::GetStatic() const
{
	return m_Static;
}

void CMesh::SetStatic( bool Static )
{
	if (m_Static != Static)
	{
		m_Static = Static;
		SafeRelease( m_hVB );
	}
}

void CMesh::ClearVertices()
{
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		m_hParts[i].clear();
	}
	SafeRelease( m_hVB );

	if (m_bUsingTranslucentIB)
	{
		ReleaseTranslucentIndexBuffers();
		m_bUsingTranslucentIB = false;
	}

	m_VertexCount = 0;
	m_GeoData.clear();
	m_VertexData.clear();
}

int32 CMesh::AddVertex()
{
	SafeRelease( m_hVB );

	uint32 const NewVertex = m_VertexCount++;

	m_GeoData.resize( m_VertexCount + 1 /*pad for unaligned SSE access*/ );

	m_VertexData.resize( m_VertexCount * m_VertexSize );

	//Initialize all to 0, except set diffuse to (1,1,1,1)
	uint8* pVertex = GetVertexPtr( NewVertex );
	memset( pVertex, 0, m_VertexSize );
	if (m_HasDiffuse)
	{
		*reinterpret_cast<uint32*>( pVertex + GetBetaMatrixBytes() ) = KWhite;
	}

    return m_VertexCount - 1;
}

void CMesh::InsertVertex( int32 Index )
{
	if (Index < 0 || Index > m_VertexCount)
	{
		throw std::out_of_range("InsertVertex index");
	}

	if (Index == m_VertexCount)
	{
		AddVertex();
		return;
	}

	SafeRelease( m_hVB );

	m_VertexData.resize( ++m_VertexCount * m_VertexSize );
	std::copy_backward( 
		GetVertexPtr( Index ), 
		GetVertexPtr( m_VertexCount - 1 ),
		GetVertexPtr( m_VertexCount ) );

	//Initialize all to 0, except set diffuse to (1,1,1,1)
	uint8* pVertex = GetVertexPtr( Index );
	memset( pVertex, 0, m_VertexSize );
	if (m_HasDiffuse)
	{
		*reinterpret_cast<uint32*>( pVertex + GetBetaMatrixBytes() ) = KWhite;
	}

	m_GeoData.insert( m_GeoData.begin() + Index, 1, GeometryData() );

	//Update mesh parts
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart* pPart = m_hParts[i][j];
			pPart->OnVertexInserted( Index );
		}
	}
}

void CMesh::RemoveVertex( int32 Index )
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "RemoveVertex index" );
	}

	SafeRelease( m_hVB );

	std::copy( 
		GetVertexPtr( Index + 1 ),
		GetVertexPtr( m_VertexCount ),
		GetVertexPtr( Index ) );

	m_VertexData.resize( --m_VertexCount * m_VertexSize );

	m_GeoData.erase( m_GeoData.begin() + Index );

	//Update or delete dependent mesh parts
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size();)
		{
			CMeshPart* pPart = m_hParts[i][j];

			if (pPart->OnVertexRemoved( Index ))
			{
				++j;
			} else {
				m_hParts[i].erase( m_hParts[i].begin() + j );
			}
		}
	}
}

void CMesh::SetVertexCount( int32 VertexCount )
{
	if (VertexCount < 0)
	{
		throw std::out_of_range( "SetVertexCount vertex count" );
	}

	if (VertexCount == m_VertexCount)
	{
		return;
	}

	SafeRelease( m_hVB );

	m_VertexData.resize( VertexCount * m_VertexSize );

	if (VertexCount > m_VertexCount)
	{
		//Added vertices

		//Initialize new vertices to 0
		uint8* pVertex = GetVertexPtr( m_VertexCount );
		memset( pVertex, 0, (VertexCount - m_VertexCount) * m_VertexSize );

		//Set diffuse to (1,1,1,1) for each vertex
		for (int32 i = m_VertexCount; i < VertexCount; ++i)
		{	
			if (m_HasDiffuse)
			{
				*reinterpret_cast<uint32*>( pVertex + GetBetaMatrixBytes() ) = KWhite;
			}
			pVertex += m_VertexSize;
		}
	} else {
		//Deleted vertices
		//Delete dependent mesh parts

		for (int32 i = 0; i < ERenderCategory::Count; ++i)
		{
			for (size_t j = 0; j < m_hParts[i].size();)
			{
				CMeshPart* pPart = m_hParts[i][j];

				if (pPart->OnVerticesTruncated( VertexCount ))
				{
					++j;
				} else {
					m_hParts[i].erase( m_hParts[i].begin() + j );
				}
			}
		}
	}

	m_GeoData.resize( VertexCount + 1 /*pad for unaligned SSE access*/ );

	m_VertexCount = VertexCount;
}

void CMesh::SetUserBitCount( int32 UserBitCount )
{
	if (UserBitCount < 0)
	{
		throw std::domain_error( "mesh user bit count" );
	}

	if (m_UserBitCount == UserBitCount)
	{
		return;
	}

	m_UserBitCount = UserBitCount;
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart* pPart = m_hParts[i][j];
			pPart->SetUserBitCount( UserBitCount );
		}
	}
}

bool CMesh::HasDiffuse() const
{
	return m_HasDiffuse;
}

void CMesh::SetHasDiffuse( bool HasDiffuse )
{
	if (m_HasDiffuse != HasDiffuse)
	{
		if (HasDiffuse)
		{
			//Inserting diffuse
			InsertVertexBytes( GetBetaMatrixBytes(),
							   GetBetaMatrixBytes() + sizeof( ARGB ) );
		} else {
			//Removing diffuse
			RemoveVertexBytes( GetBetaMatrixBytes(),
							   GetBetaMatrixBytes() + sizeof( ARGB ) );
		}

		m_HasDiffuse = HasDiffuse;

		UpdateVertexFormat();

		if (HasDiffuse)
		{
			SetDiffuse( KWhite );
		}
	}
}

void CMesh::RemoveVertexBytes( uint32 StartByte, uint32 EndByte )
{
	assert( StartByte >= 0 );
	assert( EndByte >= StartByte );
	assert( EndByte <= m_VertexSize );

	SafeRelease( m_hVB );

	uint32 NewVertexSize = m_VertexSize - (EndByte - StartByte);

	uint8 const* pSource = &m_VertexData[0];
	uint8 *pDest = &m_VertexData[0];

	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		memmove( pDest, pSource, StartByte );
		memmove( pDest + StartByte, pSource + EndByte, m_VertexSize - EndByte );
		pSource += m_VertexSize;
		pDest += NewVertexSize;
	}
	
	m_VertexData.resize( m_VertexCount * NewVertexSize );
}

void CMesh::InsertVertexBytes( uint32 StartByte, uint32 EndByte )
{
	assert( StartByte >= 0 );
	assert( EndByte >= StartByte );
	assert( StartByte <= m_VertexSize );

	SafeRelease( m_hVB );

	int32 const OldTrailingSize = m_VertexSize - StartByte;
	int32 const NewDataSize = EndByte - StartByte;
	uint32 NewVertexSize = m_VertexSize + NewDataSize;

	m_VertexData.resize( m_VertexCount * NewVertexSize );

	uint8 const* pSource = GetVertexPtr( m_VertexCount );
	uint8 *pDest = &m_VertexData[0] + NewVertexSize * m_VertexCount;

	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		//Copy old terminating data
		pDest -= OldTrailingSize;
		pSource -= OldTrailingSize;
		memmove( pDest, pSource, OldTrailingSize );

		//Create new data
		pDest -= NewDataSize;
		memset( pDest, 0, NewDataSize );

		//Copy old data
		pDest -= StartByte;
		pSource -= StartByte;
		memmove( pDest, pSource, StartByte );
	}
}

void CMesh::ZeroVertexBytes( uint32 StartByte, uint32 EndByte )
{
	assert( StartByte >= 0 );
	assert( EndByte >= StartByte );
	assert( EndByte <= m_VertexSize );

	SafeRelease( m_hVB );

	uint8 *pDest = &m_VertexData[0];

	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		memset( pDest + StartByte, 0, EndByte - StartByte );
		pDest += m_VertexSize;
	}
}

void CMesh::ClearUVs()
{
	if (m_UVCount == 0)
	{
		return;
	}

	RemoveVertexBytes( GetUVByteOffset(), m_VertexSize );
	m_UVCount = 0;

	UpdateVertexFormat();
}

int32 CMesh::AddUV()
{
	if (m_UVCount >= KMaxTextureCount)
	{
		return -1;
	}

	InsertVertexBytes( m_VertexSize, m_VertexSize + sizeof( CVector2f ) );
	++m_UVCount;

	UpdateVertexFormat();

	return m_UVCount - 1;
}

bool CMesh::InsertUV( int32 Index )
{
	if (Index < 0 || Index > m_UVCount)
	{
		throw std::out_of_range( "InsertUV index" );
	}

	if (m_UVCount >= KMaxTextureCount)
	{
		return false;
	}

	InsertVertexBytes( GetUVByteOffset() + sizeof( CVector2f ) * Index,
					   GetUVByteOffset() + sizeof( CVector2f ) * (Index + 1) );
	++m_UVCount;

	UpdateVertexFormat();

	return true;
}

void CMesh::RemoveUV( int32 Index )
{
	if (Index < 0 || Index >= m_UVCount)
	{
		throw std::out_of_range( "RemoveUV index" );
	}

	RemoveVertexBytes( GetUVByteOffset() + sizeof( CVector2f ) * Index,
				       GetUVByteOffset() + sizeof( CVector2f ) * (Index + 1) );
	--m_UVCount;

	UpdateVertexFormat();
}

bool CMesh::SetUVCount( int32 UVCount )
{
	if (UVCount < 0 || UVCount > KMaxTextureCount)
	{
		return false;
	}

	if (m_UVCount == UVCount)
	{
		return true;
	}

	if (m_UVCount > UVCount)
	{
		//Removing UV's
		RemoveVertexBytes( GetUVByteOffset() + sizeof( CVector2f ) * UVCount,
					       GetUVByteOffset() + sizeof( CVector2f ) * m_UVCount );
	} else {
		//Inserting UV's
		InsertVertexBytes( GetUVByteOffset() + sizeof( CVector2f ) * m_UVCount,
					       GetUVByteOffset() + sizeof( CVector2f ) * UVCount );
	}

	m_UVCount = UVCount;

	UpdateVertexFormat();

	return true;
}

void CMesh::ClearBetas()
{
	if (m_BetaCount == 0)
	{
		return;
	}

	RemoveVertexBytes( 0, GetBetaBytes() );
	m_BetaCount = 0;

	UpdateVertexFormat();

	UpdateBlendingMode();
}

int32 CMesh::AddBeta()
{
	if (m_BetaCount >= KMaxBetaCount)
	{
		return -1;
	}

	InsertVertexBytes( GetBetaBytes(),
					   GetBetaBytes() + sizeof( float32 ) );
	++m_BetaCount;

	UpdateVertexFormat();

	UpdateBlendingMode();

	return m_BetaCount - 1;
}

bool CMesh::InsertBeta( int32 Index )
{
	if (Index < 0 || Index > m_BetaCount)
	{
		return false;
	}

	if (m_BetaCount >= KMaxBetaCount)
	{
		return false;
	}

	InsertVertexBytes( sizeof( float32 ) * Index,
					   sizeof( float32 ) * (Index + 1) );
	++m_BetaCount;

	UpdateVertexFormat();

	UpdateBlendingMode();

	return true;
}

void CMesh::RemoveBeta( int32 Index )
{
	if (Index < 0 || Index >= m_BetaCount)
	{
		throw std::out_of_range( "RemoveBeta index" );
	}

	RemoveVertexBytes( sizeof( float32 ) * Index,
				       sizeof( float32 ) * (Index + 1) );
	--m_BetaCount;

	UpdateVertexFormat();

	UpdateBlendingMode();
}

bool CMesh::SetBetaCount( int32 BetaCount )
{
	if (BetaCount < 0 || BetaCount > KMaxTextureCount)
	{
		return false;
	}

	if (m_BetaCount == BetaCount)
	{
		return true;
	}

	if (m_BetaCount > BetaCount)
	{
		//Removing beta's
		RemoveVertexBytes( sizeof( float32 ) * BetaCount,
					       GetBetaBytes() );
	} else {
		//Inserting beta's
		InsertVertexBytes( GetBetaBytes(),
					       sizeof( float32 ) * BetaCount );
	}

	m_BetaCount = BetaCount;

	UpdateVertexFormat();

	UpdateBlendingMode();

	return true;
}

void CMesh::ClearMatrices()
{
	if (m_MatrixCount == 0)
	{
		return;
	}

	RemoveVertexBytes( GetBetaBytes(), 
					   GetBetaMatrixBytes() );
	m_MatrixCount = 0;

	UpdateVertexFormat();

	UpdateBlendingMode();
}

int32 CMesh::AddMatrix()
{
	if (m_MatrixCount >= KMaxMatrixCount)
	{
		return -1;
	}

	if (++m_MatrixCount == 1)
	{
		//First matrix
		InsertVertexBytes( GetBetaBytes(), 
						   GetBetaMatrixBytes() );

		UpdateVertexFormat();

		UpdateBlendingMode();
	}

	return m_MatrixCount - 1;
}

bool CMesh::InsertMatrix( int32 Index )
{
	if (Index < 0 || Index > m_MatrixCount)
	{
		return false;
	}

	if (m_MatrixCount >= KMaxMatrixCount)
	{
		return false;
	}

	if (m_MatrixCount == 0)
	{
		return AddMatrix() != -1;
	}

	ZeroVertexBytes( GetBetaBytes() + Index,
					 GetBetaBytes() + Index + 1 );

	++m_MatrixCount;

	return true;
}

void CMesh::RemoveMatrix( int32 Index )
{
	if (Index < 0 || Index >= m_MatrixCount)
	{
		throw std::out_of_range( "RemoveMatrix index" );
	}

	if (m_MatrixCount == 1)
	{
		ClearMatrices();
		return;
	}

	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		uint8* pVertex = GetVertexPtr( i );
		memcpy( pVertex + GetBetaBytes() + Index, 
				pVertex + GetBetaBytes() + Index + 1, 
				m_MatrixCount - Index - 1 );
		pVertex[GetBetaBytes() + m_MatrixCount - 1] = 0;
	}

	--m_MatrixCount;
}

bool CMesh::SetMatrixCount( int32 MatrixCount )
{
	if (MatrixCount < 0 || MatrixCount > KMaxMatrixCount)
	{
		return false;
	}

	if (MatrixCount)
	{
		if (!m_MatrixCount)
		{
			if (AddMatrix() == -1)
			{
				return false;
			}
		} else if (MatrixCount < m_MatrixCount) {
			//Zero out abandoned data
			ZeroVertexBytes(
				GetBetaBytes() + MatrixCount,
				GetBetaBytes() + m_MatrixCount );
		}

		m_MatrixCount = MatrixCount;

	} else {
		ClearMatrices();
	}

	return true;
}

void CMesh::SetVertexPosition( int32 Index, CVector3f const& Position )
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "SetVertexPosition index" );
	}

	//If we are rendering our internal position data, and not externally supplied data,
	//then the vertex buffer is now dirty.
	if (m_pRenderGeoData == NULL)
	{
		m_bDirtyVB = true;
	}

	m_GeoData[Index].m_Position = Position;
}

void CMesh::SetVertexPosition( int32 Index, float32 PositionX, float32 PositionY, float32 PositionZ )
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "SetVertexPosition index" );
	}

	//If we are rendering our internal position data, and not externally supplied data,
	//then the vertex buffer is now dirty.
	if (m_pRenderGeoData == NULL)
	{
		m_bDirtyVB = true;
	}

	m_GeoData[Index].m_Position = CVector3f( PositionX, PositionY, PositionZ );
}

void CMesh::SetVertexNormal( int32 Index, CVector3f const& Normal )
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "SetVertexNormal index" );
	}

	//If we are rendering our internal position data, and not externally supplied data,
	//then the vertex buffer is now dirty.
	if (m_pRenderGeoData == NULL)
	{
		m_bDirtyVB = true;
	}

	m_GeoData[Index].m_Normal = Normal;
}

void CMesh::SetVertexNormal( int32 Index, float32 NormalX, float32 NormalY, float32 NormalZ )
{
	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "SetVertexNormal index" );
	}

	//If we are rendering our internal position data, and not externally supplied data,
	//then the vertex buffer is now dirty.
	if (m_pRenderGeoData == NULL)
	{
		m_bDirtyVB = true;
	}

	m_GeoData[Index].m_Normal = CVector3f( NormalX, NormalY, NormalZ );
}

void CMesh::SetVertexDiffuse( int32 Index, ARGB Diffuse )
{
	if (!m_HasDiffuse)
	{
		throw std::runtime_error( "SetVertexDiffuse on CMesh that lacks diffuse colors" ); 
	}

	if (Index < 0 || Index >= m_VertexCount)
	{
		throw std::out_of_range( "SetVertexDiffuse index" );
	}

	m_bDirtyVB = true;

	uint32* pDiffuse = reinterpret_cast<uint32*>( GetVertexPtr( Index ) + GetBetaMatrixBytes() );
	*pDiffuse = Diffuse;
}

void CMesh::SetDiffuse( ARGB Diffuse )
{
	if (!m_HasDiffuse)
	{
		throw std::runtime_error( "SetDiffuse on CMesh that lacks diffuse colors" ); 
	}

	uint8* pDiffuse = GetVertexPtr( 0 ) + GetBetaMatrixBytes();
	for (int32 i = 0; i < m_VertexCount; ++i)
	{	
		*reinterpret_cast<uint32*>( pDiffuse ) = KWhite;
		pDiffuse += m_VertexSize;
	}
}

void CMesh::SetVertexUV( int32 Index, int32 StageIndex, CVector2f const& UV )
{
	if (Index < 0 || Index >= m_VertexCount || StageIndex < 0 || StageIndex >= m_UVCount)
	{
		throw std::out_of_range( "SetVertexUV indices" );
	}

	m_bDirtyVB = true;

	CVector2f* pUV = reinterpret_cast<CVector2f*>( GetVertexPtr( Index ) + GetUVByteOffset() );
	pUV[StageIndex] = UV;
}

void CMesh::SetVertexUV( int32 Index, int32 StageIndex, float32 U, float32 V )
{
	if (Index < 0 || Index >= m_VertexCount || StageIndex < 0 || StageIndex >= m_UVCount)
	{
		throw std::out_of_range( "SetVertexUV indices" );
	}

	m_bDirtyVB = true;

	CVector2f* pUV = reinterpret_cast<CVector2f*>( GetVertexPtr( Index ) + GetUVByteOffset() );
	pUV[StageIndex].x = U;
	pUV[StageIndex].y = V;
}

void CMesh::SetVertexBeta( int32 Index, int32 BlendIndex, float32 Beta )
{
	if (Index < 0 || Index >= m_VertexCount || BlendIndex < 0 || BlendIndex >= m_BetaCount)
	{
		throw std::out_of_range( "SetVertexBeta indices" );
	}

	m_bDirtyVB = true;

	float32* pBeta = reinterpret_cast<float32*>( GetVertexPtr( Index ) );
	pBeta[BlendIndex] = Beta;
}

void CMesh::SetVertexMatrix( int32 VertexIndex, int32 MatrixIndex, int32 VertexMatrixIndex )
{
	if (VertexIndex < 0 || VertexIndex >= m_VertexCount || MatrixIndex < 0 || MatrixIndex >= m_MatrixCount)
	{
		throw std::out_of_range( "SetVertexMatrix indices" );
	}

	m_bDirtyVB = true;

	uint8* pMatrix = GetVertexPtr( VertexIndex ) + GetBetaBytes();
	pMatrix[MatrixIndex] = static_cast<uint8>( VertexMatrixIndex );
}

void CMesh::InvertParts()
{
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart* pPart = m_hParts[i][j];
			pPart->Invert();
		}
	}
}

void CMesh::CopyVertex( int32 DestIndex, int32 SourceIndex )
{
	if (DestIndex < 0 || DestIndex >= m_VertexCount ||
		SourceIndex < 0 || SourceIndex >= m_VertexCount)
	{
		throw std::invalid_argument( "vertex index" );
	}

	SetVertexPosition( DestIndex, GetVertexPosition( SourceIndex ) );
	SetVertexNormal( DestIndex, GetVertexNormal( SourceIndex ) );

	if (m_HasDiffuse)
	{
		SetVertexDiffuse( DestIndex, GetVertexDiffuse( SourceIndex ) );
	}

	for (int32 i = 0; i < m_BetaCount; ++i)
	{
		SetVertexBeta( DestIndex, i, GetVertexBeta( SourceIndex, i ) );
	}
	for (int32 i = 0; i < m_MatrixCount; ++i)
	{
		SetVertexMatrix( DestIndex, i, GetVertexMatrix( SourceIndex, i ) );
	}
	for (int32 i = 0; i < m_UVCount; ++i)
	{
		SetVertexUV( DestIndex, i, GetVertexUV( SourceIndex, i ) );
	}
}

void CMesh::Add( CMesh const& mesh )
{
	int32 const MeshBetas = mesh.GetBetaCount();
	if (m_BetaCount < MeshBetas )
	{
		SetBetaCount( MeshBetas );
	}

	int32 const MeshMatrices = mesh.GetMatrixCount();
	if (m_MatrixCount < MeshMatrices)
	{
		SetMatrixCount( MeshMatrices );
	}

	if (!m_HasDiffuse && mesh.HasDiffuse())
	{
		SetHasDiffuse( true );
	}

	int32 const MeshTextures = mesh.GetUVCount();
	if (m_UVCount < MeshTextures )
	{
		SetUVCount( MeshTextures );
	}

	int32 const OldVertices = m_VertexCount;
	int32 const MeshVertices = mesh.GetVertexCount();
	SetVertexCount( OldVertices + MeshVertices );

	//Copy mesh's vertices into our own vertices.
	for (int32 i = 0; i < MeshVertices; ++i)
	{
		for (int32 j = 0; j < MeshBetas; ++j)
		{
			SetVertexBeta( OldVertices + i, j, mesh.GetVertexBeta( i, j ) );
		}

		for (int32 j = 0; j < MeshMatrices; ++j)
		{
			SetVertexMatrix( OldVertices + i, j, mesh.GetVertexMatrix( i, j ) );
		}

		if (mesh.HasDiffuse())
		{
			SetVertexDiffuse( OldVertices + i, mesh.GetVertexDiffuse( i ) );
		}

		for (int32 j = 0; j < MeshTextures; ++j)
		{
			SetVertexUV( OldVertices + i, j, mesh.GetVertexUV( i, j ) );
		}
	}

	//Copy geometry data for new vertices
	for (int32 i = 0; i < MeshVertices; ++i)
	{
		m_GeoData[i + OldVertices].m_Position = mesh.GetVertexPosition( i );
		m_GeoData[i + OldVertices].m_Normal   = mesh.GetVertexNormal  ( i );
	}

	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		ERenderCategory::Type RenderCategory = static_cast<ERenderCategory::Type>( i );
		for (size_t j = 0; j < mesh.GetParts( RenderCategory ).size(); ++j)
		{
			CMeshPart const* pSourcePart = mesh.GetParts( RenderCategory )[j];

			HMeshPart hDestPart = pSourcePart->Clone();
			
			//Adjust the vertex indices - make the new part using the new vertices
			int32 const SourceCount = pSourcePart->GetVertexCount();
			for (int32 j = 0; j < SourceCount; ++j)
			{
				hDestPart->SetVertex( j, hDestPart->GetVertex( j ) + OldVertices );
			}

			m_hParts[i].push_back( hDestPart );
		}
	}
}

void WTCall CMeshHelper::SetCylinder( CMesh& M, float32 Height, float32 BottomRadius, float32 TopRadius,
	int32 VerticalDivisionCount, int32 AnnularDivisionCount )
{
	M.Clear();

	if (VerticalDivisionCount < 1 || AnnularDivisionCount < 1)
	{
		return;
	}

	Height *= 0.5f;

	int32 const CapVertexCount = 2 + AnnularDivisionCount;
	int32 const CapsVertexCount = CapVertexCount * 2;

	M.SetVertexCount( CapsVertexCount + (VerticalDivisionCount + 1) * AnnularDivisionCount );

	for (int32 i = 0; i < 2 /*caps*/ + VerticalDivisionCount; ++i)
	{
		HMeshPart hMeshPart = new CMeshPart( &M );
		M.GetParts().push_back( hMeshPart );
	}

	//Bottom cap center vertex
	M.SetVertexPosition( 0, 0.0f, -Height, 0.0f );
	M.SetVertexNormal( 0, 0.0f, -1.0f, 0.0f );

	CMeshPart* pPart0 = M.GetParts()[0];
	pPart0->SetVertexCount( CapVertexCount );
	pPart0->SetPartType( CMeshPart::EType::TriFan );
	pPart0->SetVertex( 0, 0 );

	//Top cap center vertex
	M.SetVertexPosition( CapVertexCount, 0.0f, Height, 0.0f );
	M.SetVertexNormal( CapVertexCount, 0.0f, 1.0f, 0.0f );

	CMeshPart* pPart1 = M.GetParts()[1];
	pPart1->SetVertexCount( CapVertexCount );
	pPart1->SetPartType( CMeshPart::EType::TriFan );
	pPart1->SetVertex( 0, CapVertexCount );

	//Cap outer vertices
	for (int32 j = 0; j < AnnularDivisionCount; ++j)
	{
		float32 Theta = KTwoPi * j / AnnularDivisionCount;
		float32 s = sinf( Theta );
		float32 c = cosf( Theta );
		M.SetVertexPosition( 1 + j, BottomRadius * c, -Height, BottomRadius * s );
		M.SetVertexNormal( 1 + j, 0.0f, -1.0f, 0.0f );
		M.SetVertexPosition( CapVertexCount + 1 + j, TopRadius * c, Height, TopRadius * s );
		M.SetVertexNormal( CapVertexCount + 1 + j, 0.0f, 1.0f, 0.0f );

		pPart0->SetVertex( 1 + j, 1 + j );
		pPart1->SetVertex( 1 + j, CapVertexCount + AnnularDivisionCount - j );
	}

	//Last cap outer vertices - repeat the first cap outer  vertex
	pPart0->SetVertex( CapVertexCount - 1, 1 );
	pPart1->SetVertex( CapVertexCount - 1, CapVertexCount + AnnularDivisionCount );

	//Body vertices
	for (int32 i = 0; i <= VerticalDivisionCount; ++i)
	{
		float32 const YParam = i * (1.0f / VerticalDivisionCount);
		float32 const Y = (2.0f * YParam - 1) * Height; 
		float32 const Radius = BottomRadius + (TopRadius - BottomRadius) * YParam;

		for (int32 j = 0; j < AnnularDivisionCount; ++j)
		{
			float32 Theta = KTwoPi * j / AnnularDivisionCount;
			float32 s = sinf( Theta );
			float32 c = cosf( Theta );
			M.SetVertexPosition( CapsVertexCount + i * AnnularDivisionCount + j, Radius * c, Y, Radius * s );
			M.SetVertexNormal( CapsVertexCount + i * AnnularDivisionCount + j, c, 0.0f, s );
		}
	}

	//Body mesh parts
	for (int32 i = 0; i < VerticalDivisionCount; ++i)
	{
		CMeshPart* pPart = M.GetParts()[2 + i];
		pPart->SetPartType( CMeshPart::EType::TriStrip );
		pPart->SetVertexCount( AnnularDivisionCount * 2 + 2 );

		//The last 2 vertices of the mesh part are the same as the first two
		pPart->SetVertex( AnnularDivisionCount * 2 + 0, CapsVertexCount + i * AnnularDivisionCount ); 
		pPart->SetVertex( AnnularDivisionCount * 2 + 1, CapsVertexCount + (i + 1) * AnnularDivisionCount ); 

		for (int32 j = 0; j < AnnularDivisionCount; ++j)
		{
			pPart->SetVertex( j * 2 + 0, CapsVertexCount + i * AnnularDivisionCount + j );
			pPart->SetVertex( j * 2 + 1, CapsVertexCount + (i + 1) * AnnularDivisionCount + j );
		}
	}
}

void WTCall CMeshHelper::SetSphere( CMesh& M, float32 Radius,
	int32 LattitudinalDivisionCount, int32 LongitudinalDivisionCount )
{
	M.Clear();

	if (LattitudinalDivisionCount < 2 || LongitudinalDivisionCount < 3 )
	{
		return;
	}

	int32 const PoleMeshPartVertexCount = 2 + LongitudinalDivisionCount;
	int32 const RingCount = LattitudinalDivisionCount - 1;
	int32 const RingVertexCount = LongitudinalDivisionCount + 1;
	int32 const VertexCount = 1 /*pole*/ + RingCount * RingVertexCount + 1 /*pole*/;
	int32 const MeshPartCount = 1 /*cap*/ + RingCount - 1 + 1 /*cap*/;

	M.SetUVCount( 1 );
	M.SetVertexCount( VertexCount );

	for (int32 i = 0; i < MeshPartCount; ++i)
	{
		HMeshPart hMeshPart = new CMeshPart( &M );
		M.GetParts().push_back( hMeshPart );
	}

	//Bottom pole vertex
	M.SetVertexPosition( 0, 0.0f, -Radius, 0.0f );
	M.SetVertexNormal( 0, 0.0f, -1.0f, 0.0f );
	M.SetVertexUV( 0, 0, CVector2f( 0.5f, 1.0f ) );
	
	CMeshPart* pPart0 = M.GetParts()[0];
	pPart0->SetVertexCount( PoleMeshPartVertexCount );
	pPart0->SetPartType( CMeshPart::EType::TriFan );
	pPart0->SetMaterialID( 0 );
	pPart0->SetVertex( 0, 0 );

	//Top pole vertex
	M.SetVertexPosition( VertexCount - 1, 0.0f, Radius, 0.0f );
	M.SetVertexNormal( VertexCount - 1, 0.0f, 1.0f, 0.0f );
	M.SetVertexUV( VertexCount - 1, 0, CVector2f( 0.5f, 0.0f ) );
	
	CMeshPart* pPartLast = M.GetParts()[ MeshPartCount - 1 ];
	pPartLast->SetVertexCount( PoleMeshPartVertexCount );
	pPartLast->SetPartType( CMeshPart::EType::TriFan );
	pPartLast->SetMaterialID( 0 );
	pPartLast->SetVertex( 0, VertexCount - 1 );

	//Cap mesh parts
	for (int32 j = 0; j <= LongitudinalDivisionCount; ++j)
	{
		pPart0->SetVertex( 1 + j, 1 + j );
		pPartLast->SetVertex( 1 + j, VertexCount - 2 - j );
	}

	//Ring vertices
	int32 VertexIndex = 0;
	for (int32 i = 0; i < RingCount; ++i)
	{
		float32 const v = 1.0f - static_cast<float32>( 1 + i ) / (RingCount + 1);
		float32 const Phi = KPi * (0.5f - v);
		float32 const sP = sinf( Phi );
		float32 const cP = cosf( Phi );

		for (int32 j = 0; j <= LongitudinalDivisionCount; ++j)
		{
			float32 const u = static_cast<float32>( j ) / LongitudinalDivisionCount;
			float32 const Theta = KTwoPi * u;
			float32 const s = sinf( Theta );
			float32 const c = cosf( Theta );
			CVector3f Position( Radius * cP * s, Radius * sP, Radius * cP * -c );

			++VertexIndex;
			M.SetVertexPosition( VertexIndex, Position );
			M.SetVertexNormal( VertexIndex, Position );
			M.SetVertexUV( VertexIndex, 0, CVector2f( u, v ) );
		}
	}

	//Ring mesh parts
	for (int32 i = 0; i < RingCount - 1; ++i)
	{
		CMeshPart* pPart = M.GetParts()[ 1 + i ];
		pPart->SetPartType( CMeshPart::EType::TriStrip );
		pPart->SetMaterialID( 0 );
		pPart->SetVertexCount( LongitudinalDivisionCount * 2 + 2 );

		for (int32 j = 0; j <= LongitudinalDivisionCount; ++j)
		{
			pPart->SetVertex( j * 2 + 0, 1 + i * (LongitudinalDivisionCount + 1) + j );
			pPart->SetVertex( j * 2 + 1, 1 + (i + 1) * (LongitudinalDivisionCount + 1) + j );
		}
	}
}

void WTCall CMeshHelper::SetSmoothSphere( CMesh& M, float32 Radius, int32 LevelCount )
{
	M.Clear();

	if (LevelCount < 0 || LevelCount > KMaxSmoothSphereLevelCount)
	{
		return;
	}

	SetIcosahedron( M );
	for (int32 i = 0; i < LevelCount; ++i)
	{
		SphereSubdivision( M );
	}

	M.Scale( Radius );
}

void WTCall CMeshHelper::SphereSubdivision( CMesh& M )
{
	std::map<uint64, int32> BisectionMap;
	std::vector<uint64> NewVertices;
	
	int32 const OldVertexCount = M.GetVertexCount();

	for (size_t k = 0; k < M.GetParts().size(); ++k)
	{
		CMeshPart const* pPart = M.GetParts()[k];
		
		if (pPart->GetPartType() != CMeshPart::EType::TriList) continue;

		int32 const IndexCount = pPart->GetVertexCount();
		for (int32 i = 0; i + 2 < IndexCount; i += 3 )
		{
			for (int32 j = 0; j < 3; ++j)
			{
				int64 Edge;

				if (pPart->GetVertex( i + j ) > pPart->GetVertex( i + ((j + 1) % 3) ))
				{
					Edge = (static_cast<uint64>( pPart->GetVertex( i + j ) ) << 32) +
						pPart->GetVertex( i + ((j + 1) % 3) );
				} else {
					Edge = (static_cast<uint64>( pPart->GetVertex( i + ((j + 1) % 3) ) ) << 32) +
						pPart->GetVertex( i + j );
				}

				if (BisectionMap.find( Edge ) == BisectionMap.end())
				{
					BisectionMap[Edge] = OldVertexCount + static_cast<int32>(NewVertices.size());
					NewVertices.push_back( Edge );
				}
			}
		}
	}

	M.SetVertexCount( OldVertexCount + static_cast<int32>(NewVertices.size()) );
	for (int32 i = 0; i < static_cast<int32>(NewVertices.size()); ++i)
	{
		CVector3f Position =
			M.GetVertexPosition( static_cast<int32>(NewVertices[i]) ) +
			M.GetVertexPosition( static_cast<int32>(NewVertices[i] >> 32) );
		Position.Normalize();
		M.SetVertexPosition( OldVertexCount + i, Position );
		M.SetVertexNormal( OldVertexCount + i, Position );
	}

	NewVertices.clear();

	for (size_t k = 0; k < M.GetParts().size(); ++k)
	{
		CMeshPart* pPart = M.GetParts()[k];
		
		if (pPart->GetPartType() != CMeshPart::EType::TriList) continue;

		int32 const TriCount = pPart->GetVertexCount() / 3;
		int32 const NewIndexCount = 4 * 3 * TriCount;

		pPart->SetVertexCount( NewIndexCount );

		for (int32 i = 3 * TriCount - 3; i >= 0; i -= 3 )
		{
			int32 BisectionVerts[3];

			for (int32 j = 0; j < 3; ++j)
			{
				int64 Edge;

				if (pPart->GetVertex( i + j ) > pPart->GetVertex( i + ((j + 1) % 3) ))
				{
					Edge = (static_cast<int64>( pPart->GetVertex( i + j ) ) << 32) +
						pPart->GetVertex( i + ((j + 1) % 3) );
				} else {
					Edge = (static_cast<int64>( pPart->GetVertex( i + ((j + 1) % 3) ) ) << 32) +
						pPart->GetVertex( i + j );
				}

				BisectionVerts[j] = BisectionMap[Edge];
			}

			pPart->SetVertex( 4 * i + 11, BisectionVerts[2] );
			pPart->SetVertex( 4 * i + 10, BisectionVerts[0] );
			pPart->SetVertex( 4 * i + 9, pPart->GetVertex( i + 0 ) );
			pPart->SetVertex( 4 * i + 8, BisectionVerts[1] );
			pPart->SetVertex( 4 * i + 7, pPart->GetVertex( i + 1 ) );
			pPart->SetVertex( 4 * i + 6, BisectionVerts[0] );
			pPart->SetVertex( 4 * i + 5, pPart->GetVertex( i + 2 ) );
			pPart->SetVertex( 4 * i + 4, BisectionVerts[1] );
			pPart->SetVertex( 4 * i + 3, BisectionVerts[2] );
			pPart->SetVertex( 4 * i + 2, BisectionVerts[2] );
			pPart->SetVertex( 4 * i + 1, BisectionVerts[1] );
			pPart->SetVertex( 4 * i + 0, BisectionVerts[0] );
		}
	}
}

void WTCall CMeshHelper::SetTetrahedron( CMesh& M )
{
	M.Clear();

	M.SetVertexCount( 4 );

	HMeshPart hMeshPart = new CMeshPart( &M );
	M.GetParts().push_back( hMeshPart );

	M.SetVertexPosition( 0, 0.0f, 1.0f, 0.0f );
	M.SetVertexPosition( 1, 0.0f, -1.0f / 3.0f, KSqrt2 * 2.0f / 3.0f );
	M.SetVertexPosition( 2, -KSqrt2 / KSqrt3, -1.0f / 3.0f, -KSqrt2 / 3.0f );
	M.SetVertexPosition( 3, +KSqrt2 / KSqrt3, -1.0f / 3.0f, -KSqrt2 / 3.0f );
	
	for (int32 i = 0; i < M.GetVertexCount(); ++i)
	{
		M.SetVertexNormal( i, M.GetVertexPosition( i ) );
	}

	CMeshPart* pPart = M.GetParts()[0];
	pPart->SetPartType( CMeshPart::EType::TriList );
	pPart->SetVertexCount( 12 );

	pPart->SetVertex( 0, 0 );
	pPart->SetVertex( 1, 1 );
	pPart->SetVertex( 2, 2 );
	pPart->SetVertex( 3, 0 );
	pPart->SetVertex( 4, 2 );
	pPart->SetVertex( 5, 3 );
	pPart->SetVertex( 6, 0 );
	pPart->SetVertex( 7, 3 );
	pPart->SetVertex( 8, 1 );
	pPart->SetVertex( 9, 1 );
	pPart->SetVertex( 10, 3 );
	pPart->SetVertex( 11, 2 );
}

void WTCall CMeshHelper::SetIcosahedron( CMesh& M )
{
	M.Clear();

	M.SetVertexCount( 12 );

	HMeshPart hMeshPart = new CMeshPart( &M );
	M.GetParts().push_back( hMeshPart );
	
	float32 const x = 2.0f / KSqrt5;
	float32 const y = 1.0f / KSqrt5;

	M.SetVertexPosition( 0, 0.0f, -1.0f, 0.0f );
	for (uint32 i = 0; i < 10; ++i)
	{
		float32 Theta = i * (KPi / 5.0f);
		M.SetVertexPosition( i + 1, x * cosf( Theta ), i & 1 ? y : -y, x * sinf( Theta ) );
	}
	M.SetVertexPosition( 11, 0.0f, 1.0f, 0.0f );

	for (int32 i = 0; i < M.GetVertexCount(); ++i)
	{
		M.SetVertexNormal( i, M.GetVertexPosition( i ) );
	}

	CMeshPart* pPart = M.GetParts()[0];
	pPart->SetPartType( CMeshPart::EType::TriList );
	pPart->SetVertexCount( 60 );

	for (uint32 i = 0; i < 5; ++i)
	{
		uint32 const j = (i + 1) % 5;
		pPart->SetVertex( 0 + 3 * i, 0 );
		pPart->SetVertex( 1 + 3 * i, 1 + 2 * i );
		pPart->SetVertex( 2 + 3 * i, 1 + 2 * j );
		pPart->SetVertex( 15 + 3 * i, 11 );
		pPart->SetVertex( 16 + 3 * i, 2 + 2 * j );
		pPart->SetVertex( 17 + 3 * i, 2 + 2 * i );
		pPart->SetVertex( 30 + 3 * i, 1 + 2 * i );
		pPart->SetVertex( 31 + 3 * i, 2 + 2 * i );
		pPart->SetVertex( 32 + 3 * i, 1 + 2 * j );
		pPart->SetVertex( 45 + 3 * i, 2 + 2 * i );
		pPart->SetVertex( 46 + 3 * i, 2 + 2 * j );
		pPart->SetVertex( 47 + 3 * i, 1 + 2 * j );
	}
}

void WTCall CMeshHelper::SetBox( CMesh& M, float32 Width, float32 Height, float32 Depth,
		int32 XDivisionCount, int32 YDivisionCount, int32 ZDivisionCount )
{
	M.Clear();

	if (XDivisionCount < 1 || YDivisionCount < 1 || ZDivisionCount < 1)
	{
		return;
	}

	AddParallelogram( M, CVector3f( -0.5f * Width, -0.5f * Height, -0.5f * Depth ),
			  CVector3f( 0.0f, 0.0f, Depth ), CVector3f( 0.0f, Height, 0.0f ), 
			  YDivisionCount, ZDivisionCount );
	AddParallelogram( M, CVector3f( 0.5f * Width, -0.5f * Height, -0.5f * Depth ),
			  CVector3f( 0.0f, Height, 0.0f ), CVector3f( 0.0f, 0.0f, Depth ),
			  YDivisionCount, ZDivisionCount );
	AddParallelogram( M, CVector3f( -0.5f * Width, -0.5f * Height, -0.5f * Depth ),
			  CVector3f( Width, 0.0f, 0.0f ), CVector3f( 0.0f, 0.0f, Depth ), 
			  ZDivisionCount, XDivisionCount );
	AddParallelogram( M, CVector3f( -0.5f * Width, 0.5f * Height, -0.5f * Depth ),
			  CVector3f( 0.0f, 0.0f, Depth ), CVector3f( Width, 0.0f, 0.0f ),
			  ZDivisionCount, XDivisionCount );
	AddParallelogram( M, CVector3f( -0.5f * Width, -0.5f * Height, -0.5f * Depth ),
			  CVector3f( 0.0f, Height, 0.0f ), CVector3f( Width, 0.0f, 0.0f ),
			  XDivisionCount, YDivisionCount );
	AddParallelogram( M, CVector3f( -0.5f * Width, -0.5f * Height, 0.5f * Depth ),
			  CVector3f( Width, 0.0f, 0.0f ), CVector3f( 0.0f, Height, 0.0f ),
			  XDivisionCount, YDivisionCount );
}

void WTCall CMeshHelper::SetRect( CMesh& M, float32 Width, float32 Height, int32 XDivisionCount, int32 YDivisionCount )
{
	M.Clear();

	if (XDivisionCount < 1 || YDivisionCount < 1)
	{
		return;
	}

	AddParallelogram( M, CVector3f( -0.5f * Width, -0.5f * Height, 0.0f ),
			  CVector3f( Width, 0.0f, 0.0f ), CVector3f( 0.0f, Height, 0.0f ),
			  XDivisionCount, YDivisionCount );
}

void WTCall CMeshHelper::AddParallelogram( CMesh& M, CVector3f const& Origin, CVector3f const& Delta1, CVector3f const& Delta2,
					   int32 Div1Count, int32 Div2Count )
{
	//Add a rectangle with corners Origin, Origin + Delta1, Origin + Delta2, and
	//Origin + Delta1 + Delta2.

	assert( Div1Count > 0 );
	assert( Div2Count > 0 );

	M.SetUVCount( 1 );

	//Add vertices

	int32 const OldVertices = M.GetVertexCount();
	int32 const NewVertices = (Div1Count + 1) * (Div2Count + 1 );
	M.SetVertexCount( OldVertices + NewVertices );

	CVector3f Normal( Delta1.CrossProduct( Delta2 ) );
	Normal.Normalize();

	for (int32 i = 0; i <= Div1Count; ++i)
	{
		float32 const U = static_cast<float32>(i) / Div1Count;
		CVector3f const RowOrigin = Origin + Delta1 * U;

		for (int32 j = 0; j <= Div2Count; ++j)
		{
			uint32 const VertexIndex = OldVertices + i * (Div2Count + 1) + j;
			float32 const V = static_cast<float32>( j ) / Div2Count;
			M.SetVertexPosition( VertexIndex, RowOrigin + Delta2 * V );
			M.SetVertexNormal( VertexIndex, Normal );
			M.SetVertexUV( VertexIndex, 0, U, V );
		}
	}

	//Add mesh parts
	for (int32 i = 0; i < Div1Count; ++i)
	{
		HMeshPart hPart = new CMeshPart( &M );
		hPart->SetPartType( CMeshPart::EType::TriStrip );
		hPart->SetMaterialID( 0 );
		hPart->SetVertexCount( 2 * (Div2Count + 1) );

		for (int32 j = 0; j <= Div2Count; ++j)
		{
			uint32 const VertexIndex = OldVertices + i * (Div2Count + 1) + j;
			hPart->SetVertex( 2 * j + 0, VertexIndex );
			hPart->SetVertex( 2 * j + 1, VertexIndex + Div2Count + 1 );
		}

		M.GetParts().push_back( hPart );
	}
}

DWORD CMesh::GetVertexFormat() const
{
	return m_VBVertexFormat;
}

uint32 CMesh::GetVertexSize() const
{
	return m_VBVertexSize;
}

/// Get the vertex declaration.
LPDIRECT3DVERTEXDECLARATION9 CMesh::GetVertexDeclaration()
{
	return m_hVDecl;
}

LPDIRECT3DVERTEXBUFFER9 CMesh::GetVertexBuffer()
{
	return m_hVB;
}

void CMesh::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, 
				   std::vector<HMaterial> const* pMaterials )
{
	Parts& parts = m_hParts[Context.GetRenderCategory()];
	int32 PartCount = static_cast<int32>( parts.size() );
	if (PartCount == 0)
	{
		return;
	}

	if (pDevice == NULL)
	{
		throw std::invalid_argument( "device" );
	}
	SetDevice( pDevice );

    AcquireDeviceResourcesMesh(); //Set up all device resources

	//Create translucent index buffer
	if (Context.GetTranslucent() && m_bSortTranslucent && !m_bUsingTranslucentIB)
	{
		AcquireTranslucentIndexBuffers( m_pDevice );
		m_bUsingTranslucentIB = true;
	}

//	pDevice->SetFVF( m_VBVertexFormat );
	assert( m_hVDecl != NULL );
	pDevice->SetVertexDeclaration( m_hVDecl );

	assert( m_hVB != NULL );
	pDevice->SetStreamSource( 0, m_hVB, 0, m_VBVertexSize );

#if MESH_WHOLE_IB
	assert( m_hIB != NULL );
	pDevice->SetIndices( m_hIB );
#endif

	D3DVERTEXBLENDFLAGS VertexBlendFlags = D3DVBF_DISABLE;
	if (m_bUseHardwareBlending)
	{
		if (m_MatrixCount) 
		{
			if (m_MatrixCount == 1) 
			{
				VertexBlendFlags = D3DVBF_0WEIGHTS;
			} else {
				VertexBlendFlags = static_cast<D3DVERTEXBLENDFLAGS>( D3DVBF_1WEIGHTS + m_MatrixCount - 2 );
			}

			pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, true );
		}
	} else {
		pDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, false );
	}
	pDevice->SetRenderState( D3DRS_VERTEXBLEND, VertexBlendFlags );

	CMaterial::ResetCachedMaterial( Context, pDevice );

	if (Context.GetTranslucent())
	{
		EnableAlphaBlendState( pDevice, Context.GetTranslucentZWrite() );

#if SUPPORT_TRANSLUCENT_SORTING
		if (m_bSortTranslucent)
		{
			RenderTranslucentWithSorting( Context, parts, pMaterials );
			DisableAlphaBlendState( pDevice );
			return;
		}
#endif
	}

	HMeshPart* const pParts = &parts[0];
	for (int32 i = 0; i < PartCount; ++i)
	{
		CMeshPart* const pPart = pParts[i];
		assert( pPart != NULL );
		pPart->Render( pDevice, Context, pMaterials );
	}

	if (Context.GetTranslucent())
	{
		DisableAlphaBlendState( pDevice );
	}
}

void CMesh::ComputeVertexNormals()
{
	//Zero out the vertex normals
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		m_GeoData[i].m_Normal = CVector3f();
	}

	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart* pPart = m_hParts[i][j];
			pPart->AddVertexNormalContribution();
		}
	}

	//Normalize the normal vectors
	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		m_GeoData[i].m_Normal.Normalize();
	}
}

/// Calculates the sphere bounds of the internal vertex data and places it in the provided
/// Sphere structure. 
/// Note this neglects animation and also does not in general return the smallest possible 
/// sphere - it returns the smallest sphere enclosing the smallest AABox enclosing the mesh.
bool WTCall CMeshHelper::CalculateSphereBounds( CMesh const& M, CSphere& Bounds )
{
	if (M.GetVertexCount() == 0)
	{
		//Any bounding volume suffices in this case.
		Bounds.Set( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
		//Is this a failure or a success?
		return false;
	}

	CVector3f vMin = M.GetVertexPosition( 0 );
	CVector3f vMax = vMin;
	for( int32 i = 1; i < M.GetVertexCount(); ++i )
	{
		const CVector3f v = M.GetVertexPosition( i );

		if( v.x < vMin.x ) vMin.x = v.x;
		else if( v.x > vMax.x ) vMax.x = v.x;

		if( v.y < vMin.y ) vMin.y = v.y;
		else if( v.y > vMax.y ) vMax.y = v.y;

		if( v.z < vMin.z ) vMin.z = v.z;
		else if( v.z > vMax.z ) vMax.z = v.z;
	}	

	const CAABox AABox( vMin, vMax );
	AABox.GetExtSphere( Bounds );

	return true;
}

/// Scales the internal mesh data to fit within a bounding sphere of a specified unit size, optionally re-centering
/// the mesh data as needed
bool WTCall CMeshHelper::ScaleTo( CMesh& M, float32 Units, bool Center )
{
	// Calculate bounds	
	CSphere Bounds;
	bool Result = CalculateSphereBounds( M, Bounds );
	if( !Result )
	{
		return Result;
	}

	// Calculate scaling factor
	const float32 Scale = Bounds.Radius == 0.0f ? 0.0f : Units / Bounds.Radius;

	// Offset verts if centering was requested
	if( Center ) 
	{
		const CVector3f Center = Bounds.ComputeCenter( );
		M.Translate( -Center );
	}
	
	// Scale the mesh
	M.Scale( CVector3f( Scale ) );

	return true;
}

void CMesh::FillVB( uint8 pData[] ) const
{
	//Fill in vertex buffer beta values, matrix indices, and diffuse, texture coordinates

	uint8 const* pSourceVertex = &m_VertexData[0];

	if (m_bUseHardwareBlending)
	{
		int32 const VBVertexSize = m_VBVertexSize;
		int32 const VertexSize = m_VertexSize;
		assert( VBVertexSize == VertexSize + static_cast<int32>( sizeof( CVector3f ) ) * 2 );
		uint32 const Offset1 = sizeof( CVector3f );
		uint32 const BytesToCopy1 = GetBetaMatrixBytes();
		uint32 const Offset2 = sizeof( CVector3f ) * 2 + BytesToCopy1;
		uint32 const BytesToCopy2 = VertexSize - BytesToCopy1;

		int32 const IntsToCopy1 = (BytesToCopy1 >> 2) - 1;
		assert( (IntsToCopy1 + 1) * sizeof(int32) == BytesToCopy1 );
		int32 const IntsToCopy2 = (BytesToCopy2 >> 2) - 1;
		assert( (IntsToCopy2 + 1) * sizeof(int32) == BytesToCopy2 );

		int32 const VertexCount = m_VertexCount;
		for (int32 i = 0; i < VertexCount; ++i)
		{
			//Copy beta and matrix indices
			//memcpy( pData + Offset1, pSourceVertex, BytesToCopy1 ); 
			for (int32 k = IntsToCopy1; k >= 0; --k)
			{
				reinterpret_cast<int32*>( pData + Offset1 )[k] = reinterpret_cast<int32 const*>( pSourceVertex )[k];
			}

			//Copy diffuse and texture coordinates
			//memcpy( pData + Offset2, pSourceVertex + BytesToCopy1, BytesToCopy2 ); 
			for (int32 k = IntsToCopy2; k >= 0; --k)
			{
				reinterpret_cast<int32*>( pData + Offset2 )[k] = 
					reinterpret_cast<int32 const*>( pSourceVertex + BytesToCopy1 )[k];
			}

			pData += VBVertexSize;
			pSourceVertex += VertexSize;
		}
	} else {
		uint32 const SourceOffset = GetBetaMatrixBytes();
		uint32 const BytesToCopy = m_VertexSize - SourceOffset;
		uint32 const DestOffset = sizeof( CVector3f ) * 2;
		pSourceVertex += SourceOffset;
		pData += DestOffset;
		
		//Diffuse and texture coordinates should always be a multiple of 4 bytes
		int32 const IntsToCopy = (BytesToCopy >> 2) - 1;
		assert( (IntsToCopy + 1) * sizeof(int32) == BytesToCopy );

		pData += m_VertexCount * m_VBVertexSize;
		pSourceVertex += m_VertexCount * m_VertexSize;
		for (int32 i = m_VertexCount - 1; i >= 0; --i)
		{
			pData -= m_VBVertexSize;
			pSourceVertex -= m_VertexSize;

			//Copy diffuse and texture coordinates
			//memcpy( pData, pSourceVertex, BytesToCopy );
			for (int32 k = IntsToCopy; k >= 0; --k)
			{
				reinterpret_cast<int32*>( pData )[k] = reinterpret_cast<int32 const*>( pSourceVertex )[k];
			}
		}
	}
}

void CMesh::FillVBGeometry( uint8 pData[] ) const
{
	//Fill in positions and normals

	uint32 const PositionOffset = 0;

	uint32 NormalOffset = sizeof( CVector3f );
	if (m_bUseHardwareBlending)
	{
		NormalOffset += GetBetaMatrixBytes();
	}

	GeometryData const* pGeoData = m_pRenderGeoData;
	if (pGeoData == NULL)
	{
		pGeoData = &m_GeoData[0];
	}

	int32 const VBVertexSize = m_VBVertexSize;
	pData += m_VertexCount * VBVertexSize;
	pGeoData += m_VertexCount;
	for (int32 i = m_VertexCount - 1; i >= 0; --i)
	{
		pData -= VBVertexSize;
		--pGeoData;
		*reinterpret_cast<CVector3f*>( pData + PositionOffset ) = pGeoData->m_Position;
		*reinterpret_cast<CVector3f*>( pData + NormalOffset   ) = pGeoData->m_Normal;
	}
}

/// Get the total number of triangles in the mesh.
int32 CMesh::ComputeTriangleCount() const
{
	int32 TriangleCount = 0;

	for (int32 j = 0; j < ERenderCategory::Count; ++j)
	{
		size_t const PartCount = m_hParts[j].size();
		for (size_t i = 0; i < PartCount; ++i)
		{
			CMeshPart const* pPart = m_hParts[j][i];
			assert( pPart != NULL );
			TriangleCount += pPart->ComputeTriangleCount();
		}
	}

	return TriangleCount;
}

namespace
{
	bool FindProperty( std::vector<CMeshVtxPropCol> const& PropCols, int32 const PropertyType,
		int32* PropColIndex = NULL, int32* PropIndex = NULL)
	{
		int32 const NumPropCols = static_cast<int32>(PropCols.size());
		for (int32 i = 0; i < NumPropCols; ++i)
		{
			uint32 const NumProperties = PropCols[i].GetPropertyCount();
			for (uint32 j = 0; j < NumProperties; ++j)
			{
				CMeshVtxProp* pProp = PropCols[i].GetProperty( j );
				assert( pProp != NULL );
				if (pProp->GetPropType() == PropertyType)
				{
					if (PropColIndex != NULL)
					{
						*PropColIndex = i;
					}
					if (PropIndex != NULL)
					{
						*PropIndex = j;
					}
					return true;
				}
			}
		}

		if (PropColIndex != NULL)
		{
			*PropColIndex = -1;
		}

		if (PropIndex != NULL)
		{
			*PropIndex = -1;
		}

		return false;
	}

	void GetVertexProperties( std::vector<CMeshVtxPropCol> const& PropCols,
		int32 StartVertex, int32 EndVertex, int32 PropertyType, float32 pData[] )
	{
		float32 const DefaultValue = GetDefaultPropertyValue( PropertyType );

		int32 PropColIndex;
		int32 PropIndex;
		bool Result = FindProperty( PropCols, PropertyType, &PropColIndex, &PropIndex );

		if (!Result)
		{
			std::fill( &pData[0], &pData[EndVertex - StartVertex], DefaultValue );
		} else {
			CMeshVtxProp const* pProp = PropCols[PropColIndex].GetProperty( PropIndex );
			assert( pProp != NULL );
			
			for (int32 i = StartVertex; i < EndVertex; ++i)
			{
				pData[i - StartVertex] = DefaultValue;
				pProp->GetValue( i, &pData[i - StartVertex] );
			}
		}
	}

	bool EmptyVtxProperty( std::vector<CMeshVtxPropCol*>& hPropCols, CMeshVtxProp* pProp )
	{
		assert( pProp != NULL );

		CMeshFVA* pFVA = pProp->GetData();
		int32 DimensionIndex = pProp->GetDimensionIndex();

		//Leave the property there but remove any data from it
		//pPropCol->RemoveProperty( PropIndex );
		pProp->SetData( NULL );
		pProp->SetVectorIndex( 0 );
		pProp->SetDimensionIndex( 0 );

		assert( pFVA != NULL );
		pFVA->RemoveDimension( DimensionIndex );

		{
			//For each property using this FVA, adjust its dimension index if necessary
			//No need to work with m_hCreatedPropCols since only referenced properties
			//are relevant.
			size_t const NumPropCols = hPropCols.size();
			for (size_t i = 0; i < NumPropCols; ++i)
			{
				CMeshVtxPropCol* pPropCol = hPropCols[i];
				assert( pPropCol != NULL );
				uint32 const NumProperties = pPropCol->GetPropertyCount();
				for (uint32 j = 0; j < NumProperties; ++j)
				{
					CMeshVtxProp* pProp = pPropCol->GetProperty( j );
					assert( pProp != NULL );
					if (pProp->GetData() == pFVA &&
						pProp->GetDimensionIndex() > DimensionIndex)
					{
						pProp->SetDimensionIndex( pProp->GetDimensionIndex() - 1 );
					}
				}
			}
		}

		return true;
	}

	//Properties of a part other than its vertices
	struct PartInfo
	{
		MeshMaterialID m_MaterialID;
	    int32 m_Flags;
		std::vector<bool> m_UserBits;

		PartInfo( CMeshPart const* pPart )
		{
			m_MaterialID = pPart->GetMaterialID();
			m_Flags = pPart->GetFlags();
			int32 UserBitCount = pPart->GetUserBitCount();
			m_UserBits.resize( UserBitCount );
			for (int32 i = 0; i < UserBitCount; ++i)
			{
				m_UserBits[i] = pPart->GetUserBit( i );
			}
		}

		bool operator<( PartInfo const& Info ) const
		{
			if (m_MaterialID < Info.m_MaterialID)
			{
				return true;
			}

			if (m_MaterialID > Info.m_MaterialID)
			{
				return false;
			}
			
			if (m_Flags < Info.m_Flags)
			{
				return true;
			}

			if (m_Flags > Info.m_Flags)
			{
				return false;
			}

			if (m_UserBits.size() < Info.m_UserBits.size())
			{
				return true;
			}

			if (m_UserBits.size() > Info.m_UserBits.size())
			{
				return false;
			}

			size_t UserBitCount = m_UserBits.size();
			for (size_t i = 0; i < UserBitCount; ++i)
			{
				if (m_UserBits[i] < Info.m_UserBits[i])
				{
					return true;
				}

				if (m_UserBits[i] > Info.m_UserBits[i])
				{
					return false;
				}
			}

			return false; //It's not less than
		}
	};

} //anonymous namespace

class CMeshRecord
{
public:
	//Categories of properties for which different serialization settings are required.
	enum EPropCategory
	{
		KPropCategoryPositions, //Position properties
		KPropCategoryNormals, //Normal properties
		KPropCategoryUVs,       //UV properties
		KPropCategoryOther,     //Other properties such as colors
		KPropCategoryBoneIndices,	  //Bone indices (storage must be lossless)
		KPropCategoryCount, //Number of property collections to have
	};

	//List of unique FVA's
	typedef std::set<CMeshFVA*> TFVASet;

	static EPropCategory GetPropertyCategory( int32 PropertyType )
	{
		EPropCategory PropCategory = KPropCategoryOther;

		switch (PropertyType)
		{
		case KVtxPropTypePositionX:
		case KVtxPropTypePositionY:
		case KVtxPropTypePositionZ:
		case KVtxPropTypeBoneWeight1:
		case KVtxPropTypeBoneWeight2:
		case KVtxPropTypeBoneWeight3:
			PropCategory = KPropCategoryPositions;
			break;
		case KVtxPropTypeNormalX:
		case KVtxPropTypeNormalY:
		case KVtxPropTypeNormalZ:
			PropCategory = KPropCategoryNormals;
			break;
		case KVtxPropTypeBoneIndex1:
		case KVtxPropTypeBoneIndex2:
		case KVtxPropTypeBoneIndex3:
		case KVtxPropTypeBoneIndex4:
			PropCategory = KPropCategoryBoneIndices;
			break;
		}

		if (PropertyType >= KVtxPropTypeTexture1U &&
			PropertyType <= KVtxPropTypeTextureLastV)
		{
			PropCategory = KPropCategoryUVs;
		}

		return PropCategory;
	}

	CMeshRecord( CMesh* pMesh )
		: m_pMesh( pMesh )
		, m_VertexCount( pMesh->GetVertexCount() )
	{
		std::fill( m_hCreatedFVAs, m_hCreatedFVAs + KPropCategoryCount, static_cast<CMeshFVA*>( NULL ) );
		std::fill( m_hCreatedPropCols, m_hCreatedPropCols + KPropCategoryCount, static_cast<CMeshVtxPropCol*>( NULL ) );
	}

	~CMeshRecord()
	{
		for (size_t i = 0; i < m_hPropCols.size(); ++i)
		{
			delete m_hPropCols[i];
		}
		m_hPropCols.clear();
	}

private:
	CMesh* m_pMesh;
	uint32 m_VertexCount;
	HMeshFVA m_hCreatedFVAs[KPropCategoryCount];
	
	//Note: m_hCreatedPropCols and m_hPropCols should contain the same set of non-NULL
	//MeshVtxPropCol pointers.
	CMeshVtxPropCol* m_hCreatedPropCols[KPropCategoryCount]; //Collection per category value; may be NULL
	std::vector<CMeshVtxPropCol*> m_hPropCols; //List of all collections
	
public:
	void InitProperty( CMeshVtxProp* pProp )
	{
		//Make sure the property has valid data
		//(It may not if it was found to have only default values during serialization.)
		if (pProp->GetData() != NULL)
		{
			return;
		}

		int32 const PropertyType = pProp->GetPropType();
		EPropCategory const PropCategory = GetPropertyCategory( pProp->GetPropType() );

		if (m_hCreatedFVAs[PropCategory] == NULL)
		{
			//Create an FVA
			m_hCreatedFVAs[PropCategory] = new CMeshFVA();

			m_hCreatedFVAs[PropCategory]->SetNumVectors( m_VertexCount );
		}

		int32 const NumDimensions = m_hCreatedFVAs[PropCategory]->GetNumDimensions();
		m_hCreatedFVAs[PropCategory]->SetNumDimensions( NumDimensions + 1 );

		pProp->SetData( m_hCreatedFVAs[PropCategory] );
		pProp->SetVectorIndex( 0 );
		pProp->SetDimensionIndex( NumDimensions );

		//Set the values for the newly created dimension
		float32 const DefaultValue = GetDefaultPropertyValue( PropertyType );
		for (uint32 i = 0; i < m_VertexCount; ++i)
		{
			pProp->SetValue( i, DefaultValue );
		}
	}

	CMeshVtxProp* AddVtxProperty( int32 PropertyType )
	{
		if (PropertyType < 0 || PropertyType > KVtxPropTypeMax)
		{
			return NULL;
		}

		EPropCategory const PropCategory = GetPropertyCategory( PropertyType );

		if (m_hCreatedPropCols[PropCategory] == NULL)
		{
			//If we've reached the limit on the number of property collections, then fail.
			if (m_hPropCols.size() >= KMaxPropColCount)
			{
				return NULL;
			}

			//Create a property collection
			m_hCreatedPropCols[PropCategory] = new CMeshVtxPropCol();
		}
		
		int32 PropertyCount = m_hCreatedPropCols[PropCategory]->GetPropertyCount();

		//If this is the first property that we're adding to this collection,
		//then the collection will becomes an officially referenced collection.
		if (PropertyCount == 0)
		{
			m_hPropCols.push_back( m_hCreatedPropCols[PropCategory] );
		}

		CMeshVtxProp* pProp( new CMeshVtxProp() );
		pProp->SetPropType( PropertyType );
		InitProperty( pProp );

		m_hCreatedPropCols[PropCategory]->SetPropertyCount( PropertyCount + 1 );
		m_hCreatedPropCols[PropCategory]->SetProperty( PropertyCount, pProp );

		return pProp;
	}

	void EliminateDefaultProperties()
	{
		for (uint32 i = 0; i < m_hPropCols.size();)
		{
			CMeshVtxPropCol* pPropCol = m_hPropCols[i];
			assert( pPropCol != NULL );

			for (int32 j = 0; j < pPropCol->GetPropertyCount();)
			{
				CMeshVtxProp* pProp = pPropCol->GetProperty( j );
				assert( pProp != NULL );

				CMeshFVA* pFVA = pProp->GetData();
				assert( pFVA != NULL );
				int32 DimensionIndex = pProp->GetDimensionIndex();
				int32 VectorIndex = pProp->GetVectorIndex();
				float32 const DefaultValue = GetDefaultPropertyValue( pProp->GetPropType() );

				uint32 k = 0;
				for (; k < m_VertexCount; ++k)
				{
					float32 Value = pFVA->GetValue( VectorIndex + k, DimensionIndex );
					if (Value != DefaultValue)
					{
						break;
					}
				}

				if (k >= m_VertexCount)
				{
					EmptyVtxProperty( m_hPropCols, pProp );
				}

				//Even if the property was emptied, it will still be in the collection.
				//Advance to the next property.
				++j; 
			}

			if (pPropCol->GetPropertyCount() == 0)
			{
				m_hPropCols.erase( m_hPropCols.begin() + i );
			} else {
				++i; //Advance to the next property collection
			}
		}
	}

	static uint8 GetBoneIndexBitCount( CMesh* pMesh )
	{
		uint32 const VertexCount = pMesh->GetVertexCount();
		uint32 const MatrixCount = pMesh->GetMatrixCount();

		int32 MaxBoneIndex = 0;
		for (uint32 i = 0; i < VertexCount; ++i)
		{
			for (uint32 k = 0; k < MatrixCount; ++k)
			{
				int32 BoneIndex = pMesh->GetVertexMatrix( i, k );
				if (BoneIndex > MaxBoneIndex)
				{
					MaxBoneIndex = BoneIndex;	
				}
			}
		}

		//Return the number of bits required to represent bone indices.
		uint8 NumBits = 0;
		while ((1 << NumBits) <= MaxBoneIndex)
		{
			++NumBits;
			assert( NumBits != 0 );
		}

		return NumBits;
	}

	//Get a list of the unique FVA's
	//If IncludeCreatedFVA's is false, then only the actually referenced FVA's will be returned
	void GetFVASet( TFVASet& FVASet, bool IncludeCreatedFVAs = true ) const
	{
		//For each property, add the FVA to the set
		size_t const PropColCount = m_hPropCols.size();
		for (size_t i = 0; i < PropColCount; ++i)
		{
			CMeshVtxPropCol* pPropCol = m_hPropCols[i];
			assert( pPropCol != NULL );
			uint32 const PropertyCount = pPropCol->GetPropertyCount();
			for (uint32 j = 0; j < PropertyCount; ++j)
			{
				CMeshVtxProp* pProp = pPropCol->GetProperty( j );
				assert( pProp != NULL );
				CMeshFVA* pFVA = pProp->GetData();
				if (pFVA != NULL)
				{
					FVASet.insert( pFVA );
				}
			}
		}

		if (IncludeCreatedFVAs)
		{
			for (int32 i = 0; i < KPropCategoryCount; ++i)
			{
				if (m_hCreatedFVAs[i] != NULL)
				{
					FVASet.insert( m_hCreatedFVAs[i] );
				}
			}
		}
	}

	uint32 GetPropertyCount() const
	{
		uint32 Result = 0;

		size_t const PropColCount = m_hPropCols.size();
		for (size_t i = 0; i < PropColCount; ++i)
		{
			CMeshVtxPropCol* pPropCol = m_hPropCols[i];
			assert( pPropCol != NULL );
			Result += pPropCol->GetPropertyCount();
		}

		return Result;
	}

	void Write( CSerializationInfo* sInfo, CMesh::SerializationParameters* pParameters )
	{
		if (pParameters->bEliminateDefaultProperties)
		{
			EliminateDefaultProperties();
		}

		//Write the FVA's that we depend on first
		TFVASet FVASet;
		GetFVASet( FVASet, false );

		for (TFVASet::iterator it = FVASet.begin(); it != FVASet.end(); ++it)
		{
			CMeshFVA* pFVA = *it;

			assert( pFVA != NULL );

			assert( pParameters != NULL );

			//Set new serialization settings.
			if (pFVA == m_hCreatedFVAs[KPropCategoryPositions])
			{
				pFVA->SetUseVQ( pParameters->bUseVQPositions );
				pFVA->SetPrecision( pParameters->FloatPrecisionPositions );
			} else if (pFVA == m_hCreatedFVAs[KPropCategoryNormals]) {
				pFVA->SetUseVQ( pParameters->bUseVQNormals );
				pFVA->SetPrecision( pParameters->FloatPrecisionNormals );
			} else if (pFVA == m_hCreatedFVAs[KPropCategoryUVs]) {
				pFVA->SetUseVQ( pParameters->bUseVQUVs );
				pFVA->SetPrecision( pParameters->FloatPrecisionUVs );
			} else if (pFVA == m_hCreatedFVAs[KPropCategoryBoneIndices]) {
				pFVA->SetUseVQ( false );
				pFVA->SetPrecision( GetBoneIndexBitCount( m_pMesh ) );
			} else {
				pFVA->SetUseVQ( pParameters->bUseVQ );
				pFVA->SetPrecision( pParameters->FloatPrecision );
			}
		}

		size_t const PropColCount = m_hPropCols.size();
		if (PropColCount > KMaxPropColCount)
		{
			//Too many property collections to serialize this object
			throw std::out_of_range( "too many property collections" );
		}

		{
			sInfo->AddValue( "VertCount", static_cast<int32>( m_VertexCount ) );

			uint8 PropColCount8 = static_cast<uint8>( PropColCount );
			sInfo->AddValue( "PropColCount", PropColCount8 );

			for (size_t i = 0; i < PropColCount; ++i)
			{
				assert( m_hPropCols[i] != NULL );
				m_hPropCols[i]->Write( sInfo );
			}

			WriteParts( sInfo, pParameters );
		} 
	}

	void WriteParts( CSerializationInfo* sInfo, CMesh::SerializationParameters* pParameters )
	{
		if (pParameters->bBuildTriStrips)
		{
			WritePartsStripify( sInfo, pParameters );
		} else {
			WritePartsNoStripify( sInfo, pParameters );
		}
	}

	void WritePartsNoStripify( CSerializationInfo* sInfo, CMesh::SerializationParameters* pParameters )
	{
		//Assert that the number of user part bits is not too large; this should
		//have also been checked early on in serialization.
		assert( m_pMesh->GetUserBitCount() <= KMaxUserBitCount );

		uint32 PartCount = 0;
		for (int32 i = 0; i < ERenderCategory::Count; ++i)
		{
			PartCount += static_cast<uint32>( m_pMesh->GetParts( static_cast<ERenderCategory::Type>( i ) ).size() );
		}

		sInfo->AddValue( "PartCount", PartCount );
		
		uint8 UserBitCount = static_cast<uint8>( m_pMesh->GetUserBitCount() );
		sInfo->AddValue( "UserBitCount", UserBitCount );

		//Create a duplicate mesh with the reduced number of vertices.
		//This is used for the purpose of writing the mesh parts.
		//TODO to simplify this code, duplicate the entire existing mesh and 
		//perform optimizations directly on this duplicate.
		HMesh hNewMesh = new CMesh();
		hNewMesh->SetVertexCount( m_VertexCount );

		sInfo->AddContainerBegin( "Parts" );
		for (int32 i = 0; i < ERenderCategory::Count; ++i)
		{
			CMesh::Parts const& Parts = m_pMesh->GetParts( static_cast<ERenderCategory::Type>( i ) );
			for (size_t j = 0; j < Parts.size(); ++j)
			{
				//Create a duplicate of the part so that we can alter it
				HMeshPart hNewPart = new CMeshPart( *Parts[j] );
				hNewPart->SetMesh( hNewMesh );
				
				//Remap the vertices
				for (int32 k = 0; k < hNewPart->GetVertexCount(); ++k)
				{
					hNewPart->SetVertex( k, hNewPart->GetVertex( k ) );
				}

				sInfo->AddValue( "item", *hNewPart );
			}
		}
		sInfo->AddContainerEnd();
	}

	void WritePartsStripify( CSerializationInfo* sInfo, CMesh::SerializationParameters* pParameters )
	{
		//TODO use RemapIndices() to improve spatial locality of vertex buffer
		//and optimize rendering.

		//Create a duplicate mesh with the reduced number of vertices.
		//This is used for the purpose of writing the mesh parts.
		//TODO to simplify this code, duplicate the entire existing mesh and 
		//perform optimizations directly on this duplicate.
		HMesh hNewMesh = new CMesh();
		hNewMesh->SetVertexCount( m_VertexCount );

		//Sort the parts based on material ID and flags.
		//For each group of parts construct a tri list.
		//Stripify each tri-list and write the new parts out.

		std::list<HMeshPart> OutParts;

		typedef std::map< PartInfo, std::vector<CMeshPart const*> > MyMap;
		MyMap PointMap;
		MyMap LineMap;
		MyMap TriMap;

		for (int32 j = 0; j < ERenderCategory::Count; ++j)
		{
			CMesh::Parts const& Parts = m_pMesh->GetParts( static_cast<ERenderCategory::Type>( j ) );
			size_t PartCount = Parts.size();
			for (size_t i = 0; i < PartCount; ++i )
			{
				CMeshPart const* pPart = Parts[i];

				PartInfo Info( pPart );

				MyMap* pMap = &TriMap;
				switch (pPart->GetPartType())
				{
					default:
						assert( false );
						continue;

					case CMeshPart::EType::PointList:
						pMap = &PointMap;
						break;

					case CMeshPart::EType::LineList:
						pMap = &LineMap;
						break;

					case CMeshPart::EType::LineStrip:
						//Line stripping not implemented currently; just write the line strip out.
						{
							HMeshPart hNewPart = new CMeshPart( *pPart );
							hNewPart->SetMesh( hNewMesh );
							int32 const PartVertexCount = hNewPart->GetVertexCount();
							for (int32 j = 0; j < PartVertexCount; ++j)
							{
								hNewPart->SetVertex( j, pPart->GetVertex( j ) );
							}
							OutParts.push_back( hNewPart );
						}
						continue;

					case CMeshPart::EType::TriList:
					case CMeshPart::EType::TriStrip:
					case CMeshPart::EType::TriFan:
					case CMeshPart::EType::QuadList:
					case CMeshPart::EType::TriStripInverted:
						break;
				}

				(*pMap)[Info].push_back( pPart );
			}
		}

		//Determine the cache size we will optimize for when generating tri-strips
		uint32 const VertexCacheSize = pParameters->VertexCacheSize;

		//Combine compatible points lists into point lists
		for (MyMap::iterator it = PointMap.begin(); it != PointMap.end(); ++it)
		{
			std::vector<unsigned int> indices;

			PartInfo const* const pPartInfo = &it->first;
			std::vector<CMeshPart const*> const& Parts = it->second;
			size_t const PartCount = Parts.size();
			for (size_t i = 0; i < PartCount; ++i)
			{
				CMeshPart const* pPart = Parts[i];
				int32 const PartVertexCount = pPart->GetVertexCount();
				for (int32 j = 0; j < PartVertexCount; ++j)
				{
					int32 Index = pPart->GetVertex( j );
					indices.push_back( Index );
				}
			}
			
			HMeshPart hNewPart = new CMeshPart( hNewMesh );

			//Copy data from part info into part
			hNewPart->SetMaterialID( pPartInfo->m_MaterialID );
			hNewPart->SetFlags( pPartInfo->m_Flags );
			assert( static_cast<uint32>( hNewPart->GetUserBitCount() ) == pPartInfo->m_UserBits.size() );
			for (uint32 i = 0; i < pPartInfo->m_UserBits.size(); ++i)
			{
				hNewPart->SetUserBit( i, pPartInfo->m_UserBits[i] ? true : false );
			}

			hNewPart->SetPartType( CMeshPart::EType::PointList );
			hNewPart->SetVertexCount( static_cast<int32>( indices.size() ) );
			for (size_t i = 0; i < indices.size(); ++i)
			{
				hNewPart->SetVertex( static_cast<int32>( i ), indices[i] );
			}

			OutParts.push_back( hNewPart );
		}

		//Combine compatible line lists into line lists
		for (MyMap::iterator it = LineMap.begin(); it != LineMap.end(); ++it)
		{
			std::vector<unsigned int> indices;

			PartInfo const* const pPartInfo = &it->first;
			std::vector<CMeshPart const*> const& Parts = it->second;
			size_t const PartCount = Parts.size();
			for (size_t i = 0; i < PartCount; ++i)
			{
				CMeshPart const* pPart = Parts[i];
				int32 const ValidPartVertexCount = pPart->GetVertexCount() & ~1;
				for (int32 j = 0; j < ValidPartVertexCount; ++j)
				{
					int32 Index = pPart->GetVertex( j );
					indices.push_back( Index );
				}
			}
			
			HMeshPart hNewPart = new CMeshPart( hNewMesh );

			//Copy data from part info into part
			hNewPart->SetMaterialID( pPartInfo->m_MaterialID );
			hNewPart->SetFlags( pPartInfo->m_Flags );
			assert( static_cast<uint32>( hNewPart->GetUserBitCount() ) == pPartInfo->m_UserBits.size() );
			for (uint32 i = 0; i < pPartInfo->m_UserBits.size(); ++i)
			{
				hNewPart->SetUserBit( i, pPartInfo->m_UserBits[i] ? true : false );
			}

			hNewPart->SetPartType( CMeshPart::EType::LineList );
			hNewPart->SetVertexCount( static_cast<int32>( indices.size() ) );
			for (uint32 i = 0; i < indices.size(); ++i)
			{
				hNewPart->SetVertex( i, indices[i] );
			}

			OutParts.push_back( hNewPart );
		}

		for (MyMap::iterator it = TriMap.begin(); it != TriMap.end(); ++it)
		{
			//List of vertices, three at a time (representing a triangle), that is
			//passed into the tri-stripper.
	#ifdef USE_NVTRISTRIP
			std::vector<unsigned int> indices;
	#else
			triangle_stripper::tri_stripper::indices indices;
	#endif
		
			std::vector<CMeshPart const*>& Parts = it->second;

			size_t PartCount = Parts.size();
			for (size_t i = 0; i < PartCount; ++i)
			{
				CMeshPart const* pPart = Parts[i];
				int32 const PartVertexCount = pPart->GetVertexCount();
				
				switch (pPart->GetPartType())
				{
					default:
						assert( false );
						continue;

					case CMeshPart::EType::TriList:	
						{
							int32 ValidPartVertexCount = PartVertexCount - (PartVertexCount % 3);
							for (int32 i = 0; i < ValidPartVertexCount; ++i)
							{
								int32 Index = pPart->GetVertex( i );
								indices.push_back( Index );
							}
						}
						break;

					case CMeshPart::EType::TriStrip:
						for (int32 i = 2; i < PartVertexCount; ++i)
						{
							int32 Indices[3];
							Indices[0] = pPart->GetVertex( i - 2 + (i & 1) );
							Indices[1] = pPart->GetVertex( i - 1 - (i & 1) );
							Indices[2] = pPart->GetVertex( i );

							indices.push_back( Indices[0] );
							indices.push_back( Indices[1] );
							indices.push_back( Indices[2] );
						}
						break;

					case CMeshPart::EType::TriFan:
						if (PartVertexCount > 0)
						{
							int32 Indices[3];
							Indices[0] = pPart->GetVertex( 0 );
							for (int32 i = 2; i < PartVertexCount; ++i)
							{
								Indices[1] = pPart->GetVertex( i - 1 );
								Indices[2] = pPart->GetVertex( i );
								indices.push_back( Indices[0] );
								indices.push_back( Indices[1] );
								indices.push_back( Indices[2] );
							}
						}
						break;

					case CMeshPart::EType::QuadList:
						for (int32 i = 0; i + 3 < PartVertexCount; i += 4)
						{
							int32 Indices[4];
							Indices[0] = pPart->GetVertex( i );
							Indices[1] = pPart->GetVertex( i + 1 );
							Indices[2] = pPart->GetVertex( i + 2 );
							Indices[3] = pPart->GetVertex( i + 3 );

							indices.push_back( Indices[0] );
							indices.push_back( Indices[1] );
							indices.push_back( Indices[2] );

							//Second triangle
							indices.push_back( Indices[2] );
							indices.push_back( Indices[3] );
							indices.push_back( Indices[0] );
						}
						break;

					case CMeshPart::EType::TriStripInverted:
						for (int32 i = 2; i < PartVertexCount; ++i)
						{
							int32 Indices[3];
							Indices[0] = pPart->GetVertex( i - 1 - (i & 1) );
							Indices[1] = pPart->GetVertex( i - 2 + (i & 1) );
							Indices[2] = pPart->GetVertex( i );

							indices.push_back( Indices[0] );
							indices.push_back( Indices[1] );
							indices.push_back( Indices[2] );
						}
						break;
				}
			}

			//Remove degenerate triangles
			for (int32 i = 2; i < static_cast<int32>( indices.size() ); )
			{
				if (indices[i] == indices[i - 2] || indices[i] == indices[i - 1] || indices[i - 1] == indices[i - 2])
				{
					indices.erase( indices.begin() + (i - 2), indices.begin() + (i + 1) );
				} else {
					i += 3;
				}
			}

			//Stripify the triangle list.
	#ifdef USE_NVTRISTRIP
			bool StripifyOk = false;
			unsigned int GroupCount = 0;
			NvTriStrip::PrimitiveGroup* pPrimGroups = NULL;
	#else
			triangle_stripper::tri_stripper::primitives_vector Primitives;
	#endif

			try
			{
	#ifdef USE_NVTRISTRIP
				NvTriStrip::SetCacheSize( VertexCacheSize );
				NvTriStrip::SetMinStripSize( 3 );

				bool bValidateEnabled = false;
	#ifndef NDEBUG
	//			bValidateEnabled = true; //This usually causes failure!
	#endif
					
				StripifyOk = NvTriStrip::GenerateStrips(
					&indices[0], static_cast<uint32>( indices.size() ), &pPrimGroups, &GroupCount, bValidateEnabled );
	#else
				triangle_stripper::tri_stripper stripper( indices );
				stripper.SetCacheSize( VertexCacheSize );
				stripper.SetMinStripSize( 3 );
			
				stripper.Strip( &Primitives );
	#endif
				
			} catch (...) {
	#ifdef USE_NVTRISTRIP
				//Looking at the NvTriStrip code, perhaps this could happen because of duplicate 
				//triangles or more than two triangles sharing an edge?
	#else
				//Tri-stripping failed. Write parts without stripifying.
				//First free unneeded memory.
				Primitives.clear();

				OutParts.clear();

				WritePartsNoStripify( pParameters );
				return;
	#endif
			}

	#ifdef USE_NVTRISTRIP
			if (!StripifyOk)
			{
				delete[] pPrimGroups;

				OutParts.clear();

				//Tri-stripping failed. Write parts without stripifying.
				WritePartsNoStripify( sInfo, pParameters );
				return;
			}
	#endif

			PartInfo const* pPartInfo = &it->first;

			//Create a part for each tri-strip
	#ifdef USE_NVTRISTRIP
			for (uint32 i = 0; i < GroupCount; ++i)
			{
				NvTriStrip::PrimitiveGroup& Group = pPrimGroups[i];
				int32 const PartVertexCount = Group.numIndices;
	#else
			for (int32 i = 0; i < Primitives.size(); ++i)
			{
				triangle_stripper::tri_stripper::primitives& Prim = Primitives[i];
				int32 const PartVertexCount = Prim.m_Indices.size();
	#endif

				//Don't write degenerate primitives
				if (PartVertexCount < 3)
				{
					continue;
				}

				HMeshPart hNewPart = new CMeshPart( hNewMesh );

				//Copy data from part info into part
				hNewPart->SetMaterialID( pPartInfo->m_MaterialID );
				hNewPart->SetFlags( pPartInfo->m_Flags );
				assert( static_cast<uint32>( hNewPart->GetUserBitCount() ) == pPartInfo->m_UserBits.size() );
				for (uint32 i = 0; i < pPartInfo->m_UserBits.size(); ++i)
				{
					hNewPart->SetUserBit( i, pPartInfo->m_UserBits[i] ? true : false );
				}

				CMeshPart::EType::Type PartType = CMeshPart::EType::TriStrip;

	#ifdef USE_NVTRISTRIP
				switch (Group.type)
	#else
				switch (Prim.m_Type)
	#endif
				{
				default:
					assert( false );
					continue;

	#ifdef USE_NVTRISTRIP
				case NvTriStrip::PT_STRIP:
					PartType = CMeshPart::EType::TriStrip; break;

				case NvTriStrip::PT_LIST:
					PartType = CMeshPart::EType::TriList; break;

				case NvTriStrip::PT_FAN:
					PartType = CMeshPart::EType::TriFan; break;
	#else
				case triangle_stripper::tri_stripper::PT_Triangle_Strip:
					PartType = CMeshPart::EType::TriStrip; break;

				case triangle_stripper::tri_stripper::PT_Triangles:
					PartType = CMeshPart::EType::TriList; break;
	#endif
				}

				hNewPart->SetPartType( PartType );
	#ifdef USE_NVTRISTRIP
				hNewPart->SetVertexCount( PartVertexCount );
				for (int32 i = 0; i < PartVertexCount; ++i)
				{
					hNewPart->SetVertex( i, Group.indices[i] );
				}
	#else
				hNewPart->SetVertexCount( PartVertexCount );
				for (int32 i = 0; i < PartVertexCount; ++i)
				{
					hNewPart->SetVertex( i, Prim.m_Indices[i] );
				}
	#endif

				OutParts.push_back( hNewPart );
			}

	#ifdef USE_NVTRISTRIP
			delete[] pPrimGroups;
	#endif
		}

		uint32 PartCount = static_cast<uint32>( OutParts.size() );
		sInfo->AddValue( "PartCount", PartCount );

		//Assert that the number of user part bits is not too large; this should
		//have also been checked early on in serialization.
		assert( m_pMesh->GetUserBitCount() <= KMaxUserBitCount );
		
		uint8 UserBitCount = static_cast<uint8>( m_pMesh->GetUserBitCount() );
		sInfo->AddValue( "UserBitCount", UserBitCount );

		sInfo->AddContainerBegin( "Parts" );
		for (std::list<HMeshPart>::iterator it = OutParts.begin(); it != OutParts.end(); ++it)
		{
			sInfo->AddValue( "item", **it );
		}
		sInfo->AddContainerEnd();
	}
};

void CMesh::GetObjectData( CSerializationInfo* sInfo, CSerializationContext* sContext ) const
{
	bool bCreatedParameters = false;
	SerializationParameters* pParameters = reinterpret_cast<SerializationParameters*>( 
		sContext->GetProperties()->GetValue<uint64>( "MeshParameters", NULL ) );
	if (pParameters == NULL)
	{
		++bCreatedParameters;
		pParameters = new SerializationParameters();
		sContext->GetProperties()->SetValue( "MeshParameters", reinterpret_cast<uint64>( pParameters ) );
	}

	try
	{
		if (GetUserBitCount() > KMaxUserBitCount)
		{
			//Too many user part bits to serialize this object
			throw std::out_of_range( "too many mesh part user bits" );
		}

		//Create a MeshRecord object, which is filled in, optimized, and then written out.
		//This is necessary because we need seperate storage for the data to be optimized.
		//This also lets us work with structures of arrays, which are more natural for compression 
		//than the arrays of structures like the Mesh vertices.
		CMeshRecord Record( const_cast<CMesh*>( this ) );
	    
		//Create position and normal properties
		{
			CMeshVtxProp* const pPropX = Record.AddVtxProperty( KVtxPropTypePositionX );
			CMeshVtxProp* const pPropY = Record.AddVtxProperty( KVtxPropTypePositionY );
			CMeshVtxProp* const pPropZ = Record.AddVtxProperty( KVtxPropTypePositionZ );
			for (int32 j = 0; j < m_VertexCount; ++j)
			{
				CVector3f const Position = GetVertexPosition( j );
				pPropX->SetValue( j, Position.x );
				pPropY->SetValue( j, Position.y );
				pPropZ->SetValue( j, Position.z );
			}

			CMeshVtxProp* const pPropNX = Record.AddVtxProperty( KVtxPropTypeNormalX );
			CMeshVtxProp* const pPropNY = Record.AddVtxProperty( KVtxPropTypeNormalY );
			CMeshVtxProp* const pPropNZ = Record.AddVtxProperty( KVtxPropTypeNormalZ );
			for (int32 j = 0; j < m_VertexCount; ++j)
			{
				CVector3f const Normal = GetVertexNormal( j );
				pPropNX->SetValue( j, Normal.x );
				pPropNY->SetValue( j, Normal.y );
				pPropNZ->SetValue( j, Normal.z );
			}
		}

		//Create bone index properties
		assert( m_MatrixCount <= KMaxMatrixCount );
		for (int32 i = 0; i < m_MatrixCount; ++i)
		{
			CMeshVtxProp* const pProp = Record.AddVtxProperty( KVtxPropTypeBoneIndex1 + (i << 1) );
			for (int32 j = 0; j < m_VertexCount; ++j)
			{
				pProp->SetValue( j, static_cast<float32>( GetVertexMatrix( j, i ) ) );
			}
		}
		
		assert( m_BetaCount <= KMaxBetaCount );
		for (int32 i = 0; i < m_BetaCount; ++i)
		{
			CMeshVtxProp* const pProp = Record.AddVtxProperty( KVtxPropTypeBoneWeight1 + (i << 1) );
			for (int32 j = 0; j < m_VertexCount; ++j)
			{
				pProp->SetValue( j, GetVertexBeta( j, i ) );
			}
		}

		assert( m_UVCount < KMaxTextureCount );
		for (int32 i = 0; i < m_UVCount; ++i)
		{
			CMeshVtxProp* const pPropU = Record.AddVtxProperty( KVtxPropTypeTexture1U + (i << 1) );
			CMeshVtxProp* const pPropV = Record.AddVtxProperty( KVtxPropTypeTexture1V + (i << 1) );
			for (int32 j = 0; j < m_VertexCount; ++j)
			{
				CVector2f UV = GetVertexUV( j, i );
				pPropU->SetValue( j, UV.x );
				pPropV->SetValue( j, UV.y );
			}
		}

		if (m_HasDiffuse)
		{
			CMeshVtxProp* const pPropA = Record.AddVtxProperty( KVtxPropTypeColorA );
			CMeshVtxProp* const pPropR = Record.AddVtxProperty( KVtxPropTypeColorR );
			CMeshVtxProp* const pPropG = Record.AddVtxProperty( KVtxPropTypeColorG );
			CMeshVtxProp* const pPropB = Record.AddVtxProperty( KVtxPropTypeColorB );
			for (int32 j = 0; j < m_VertexCount; ++j)
			{
				uint32 Diffuse = GetVertexDiffuse( j );
				pPropA->SetValue( j, static_cast<float32>( Diffuse >> 24 ) );
				pPropR->SetValue( j, static_cast<float32>( (Diffuse >> 16) & 0xff ) );
				pPropG->SetValue( j, static_cast<float32>( (Diffuse >> 8) & 0xff ) );
				pPropB->SetValue( j, static_cast<float32>( Diffuse & 0xff ) );
			}
		}

		Record.Write( sInfo, pParameters );
	} catch (...) {
		if (bCreatedParameters)
		{
			delete pParameters;
			sContext->GetProperties()->Erase( "MeshParameters" );
		}

		throw;
	}

	//TODO fix this - failure to delete pParameters causes a leak.
	//This can't be deleted here due to its being needed by other 
	//serialized objects. This could be fixed by adding the ability 
	//for the serialization context or serialization system to 
	//handle ownership.
	//Note: having all dependent objects serialized (e.g. to temporary
	//streams) before this point in the code was reached would also
	//be a solution; std::auto_ptr<> could be used to ensure destruction
	//in the face of exceptions.
#if 0
	if (bCreatedParameters)
	{
		delete pParameters;
		sContext->GetProperties()->Remove( "MeshParameters" );
	}
#endif
}

/// Compute an estimate of the serialized size in bytes.
//TODO rewrite this
int32 CMesh::EstimateSerializedSize( SerializationParameters* pParameters ) const
{
	return 0;
}

void CMesh::SetObjectData( CSerializationInfo* sInfo, CSerializationContext* sContext )
{
	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	if (pDevice == NULL)
	{
		throw std::invalid_argument( "NULL D3DDevice in serialization context" );
	}
	SetDevice( pDevice );

	if (sInfo->GetObjectFormatVersion() < 2)
	{
		throw std::runtime_error( "obsolete mesh file version" );
	}

	//Clear all everything except for materials
	m_HasDiffuse = false;
	m_UVCount = 0;
	m_BetaCount = 0;
	m_MatrixCount = 0;

	UpdateVertexFormat();

	ClearVertices();
	m_UserBitCount = 0;

	int32 VertCount;
	sInfo->GetValue( "VertCount", VertCount );
	if (VertCount < 0)
	{
		throw std::invalid_argument( "corrupt mesh stream" );
	}
	SetVertexCount( VertCount );

	uint8 PropColCount;
	sInfo->GetValue( "PropColCount", PropColCount );

	//Read vertex properties
	PropCols.resize( PropColCount );
	for (int32 i = 0; i < PropColCount; ++i)
	{
		PropCols[i].Read( sInfo );
	}

	uint32 PartCount;
	sInfo->GetValue( "PartCount", PartCount );
	if (PartCount < 0)
	{
		throw std::invalid_argument( "corrupt mesh stream" );
	}

	uint8 UserBitCount;
	sInfo->GetValue( "UserBitCount", UserBitCount );
	SetUserBitCount( UserBitCount );

	sInfo->GetContainerBegin( "Parts" );
	for (uint32 i = 0; i < PartCount; ++i)
	{
		HMeshPart hPart( new CMeshPart( this ) );
		sInfo->GetValue( "item", *hPart );

		ERenderCategory::Type RenderCategory = ERenderCategory::Opaque;
		
		uint32 const RenderFlags = hPart->GetFlags() & (CMeshPart::EFlags::RenderFirst | CMeshPart::EFlags::RenderLast);

		if (RenderFlags == CMeshPart::EFlags::RenderFirst) {
			RenderCategory = ERenderCategory::PreOpaque;
		} else if (RenderFlags == CMeshPart::EFlags::RenderLast || hPart->GetFlags() & CMeshPart::EFlags::Translucent) {
			RenderCategory = ERenderCategory::PostOpaque;
		}

		m_hParts[RenderCategory].push_back( hPart );
	}
	sInfo->GetContainerEnd();
}

void CMesh::FinalizeDeserialization( CSerializationInfo *sInfo, CSerializationContext* sContext )
{
	//_BaseClass::FinalizeDeserialization( sInfo, sContext );

	assert( m_MatrixCount == 0 ); //Should be true as a result of Clear()
	for (uint32 i = 0; i < KMaxMatrixCount; ++i)
	{
		if (FindProperty( PropCols, KVtxPropTypeBoneIndex1 + (i << 1) ))
		{
			m_MatrixCount = i + 1;
		}
	}

	assert( m_BetaCount == 0 ); //Should be true as a result of Clear()
	for (uint32 i = 0; i < KMaxBetaCount; ++i)
	{
		if (FindProperty( PropCols, KVtxPropTypeBoneWeight1 + (i << 1) ))
		{
			m_BetaCount = i + 1;
		}
	}

	assert( !m_HasDiffuse );
	if (FindProperty( PropCols, KVtxPropTypeColorR ) ||
		FindProperty( PropCols, KVtxPropTypeColorG ) ||
		FindProperty( PropCols, KVtxPropTypeColorB ) ||
		FindProperty( PropCols, KVtxPropTypeColorA ))
	{
		m_HasDiffuse = true;
	}

	assert( m_UVCount == 0 ); //Should be true as a result of Clear()
	for (uint32 i = 0; i < KMaxTextureCount; ++i)
	{
		if (FindProperty( PropCols, KVtxPropTypeTexture1U + (i << 1)) ||
			FindProperty( PropCols, KVtxPropTypeTexture1V + (i << 1)))
		{
			m_UVCount = i + 1;
		}
	}

	UpdateVertexFormat();

	m_VertexData.resize( m_VertexCount * m_VertexSize );

	std::vector<float32> Data( m_VertexCount );

	uint8* pVertexData;

	for (int32 j = 0; j < m_BetaCount; ++j)
	{
		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeBoneWeight1 + (j << 1), &Data[0] );
		pVertexData = &m_VertexData[0] + sizeof( float32 ) * j;
		for (int32 i = 0; i < m_VertexCount; ++i, pVertexData += m_VertexSize)
			*reinterpret_cast<float32*>(pVertexData) = Data[i];
	}
	
	for (int32 j = 0; j < m_MatrixCount; ++j)
	{
		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeBoneIndex1 + (j << 1), &Data[0] );
		pVertexData = &m_VertexData[0] + (GetBetaBytes() + j);
		for (int32 i = 0; i < m_VertexCount; ++i, pVertexData += m_VertexSize)
			*pVertexData = static_cast<uint8>( Float32ToInt32( Data[i] ) );
	}

	if (m_HasDiffuse)
	{
		//For each wtMesh property (R, G, B, and A)
		for (int32 c = 0; c < 4; ++c)
		{
			if (FindProperty( PropCols, KVtxPropTypeColorR + c ))
			{
				GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeColorR + c, &Data[0] );

				pVertexData = &m_VertexData[0] + GetBetaMatrixBytes();
				pVertexData += (2 - c) & 3; //Associate R with byte 2, G with byte 1, B with byte 0, and A with byte 3

				for (int32 i = 0; i < m_VertexCount; ++i, pVertexData += m_VertexSize)
				{
					uint32 I = Float32ToInt32( Data[i] );
					if (I < 0) I = 0; else if (I > 255) I = 255;
					*pVertexData = static_cast<uint8>( I );
				}
			}
		}
	}

	CVector2f* pUV = reinterpret_cast<CVector2f*>( &m_VertexData[0] + GetUVByteOffset() );

	for (int32 j = 0; j < m_UVCount; ++j)
	{
		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeTexture1U + (j << 1), &Data[0] );
		pVertexData = reinterpret_cast<uint8*>( &pUV[j].x );
		for (int32 i = 0; i < m_VertexCount; ++i, pVertexData += m_VertexSize)
			*reinterpret_cast<float32*>(pVertexData) = Data[i];

		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeTexture1V + (j << 1), &Data[0] );
		pVertexData = reinterpret_cast<uint8*>( &pUV[j].y );
		for (int32 i = 0; i < m_VertexCount; ++i, pVertexData += m_VertexSize)
			*reinterpret_cast<float32*>(pVertexData) = Data[i];
	}

	//Fill in geometry data
	m_GeoData.resize( m_VertexCount + 1 /*pad for unaligned SSE access*/ );
	{
		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypePositionX, &Data[0] );
		for (int32 i = 0; i < m_VertexCount; ++i)
			m_GeoData[i].m_Position.x = Data[i];

		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypePositionY, &Data[0] );
		for (int32 i = 0; i < m_VertexCount; ++i)
			m_GeoData[i].m_Position.y = Data[i];

		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypePositionZ, &Data[0] );
		for (int32 i = 0; i < m_VertexCount; ++i)
			m_GeoData[i].m_Position.z = Data[i];

		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeNormalX, &Data[0] );
		for (int32 i = 0; i < m_VertexCount; ++i)
			m_GeoData[i].m_Normal.x = Data[i];

		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeNormalY, &Data[0] );
		for (int32 i = 0; i < m_VertexCount; ++i)
			m_GeoData[i].m_Normal.y = Data[i];

		GetVertexProperties( PropCols, 0, m_VertexCount, KVtxPropTypeNormalZ, &Data[0] );
		for (int32 i = 0; i < m_VertexCount; ++i)
			m_GeoData[i].m_Normal.z = Data[i];
	}

	Data.clear();

	//If no vertex normals are present in the mesh, then compute them manually
	if (!FindProperty( PropCols, KVtxPropTypeNormalX ) &&
		!FindProperty( PropCols, KVtxPropTypeNormalY ) &&
		!FindProperty( PropCols, KVtxPropTypeNormalZ ) )
	{
		ComputeVertexNormals();
	}

	PropCols.clear(); //No longer needed.

	UpdateBlendingMode();
}

uint32 WTCall CMesh::GetClassFormatVersion() {return 2;}

CMesh::SerializationParameters::SerializationParameters()
{
	VertexCacheSize = 32; //256
    FloatPrecision = 8;
    FloatPrecisionPositions = 16;
	FloatPrecisionNormals   = 10;

	//Unfortunately, the number of UV bits needs to be large due
	//to the possibility of very large UV values. The distortion
	//here needs to be much less than 1.
	FloatPrecisionUVs = 16;

	bEliminateDefaultProperties = true;
    bUseHuffmanCompression		= true;
	bUseIndexedData				= true;
    bBuildTriStrips				= true;
    bUseVQ						= true;
    bUseVQPositions				= false;
    bUseVQNormals				= false;
    bUseVQUVs					= false;
}

inline void HashAdd( int32& Hash, float32 Value, float32 Tolerance )
{
	//Round to multiple of 2*tolerance so that nearby but not
	//identical vertices have the same hash value and can be compared.
	Value = Float32ToInt32( Value / (2.0f * Tolerance) ) * 2.0f * Tolerance;
	Hash ^= *reinterpret_cast<int32*>( &Value );
}

//Functor to help indirectly sort vertices
struct MyPredicate : public std::greater<int32>
{
	int32 const* m_pValues;
	MyPredicate( std::vector<int32> const& Values ) : m_pValues( &Values[0] ) {};
	bool operator()(int32 const& i, int32 const& j) const
	{
		return m_pValues[i] < m_pValues[j];
	}
};

float32 const KPositionTolerance = 0.01f;
float32 const KNormalTolerance = 0.01f;
float32 const KColorTolerance = 2.55f;
float32 const KBetaTolerance = 0.01f;
float32 const KUVTolerance = 0.01f;

bool VerticesEqual( CMesh const& Mesh, int32 Vertex1, int32 Vertex2 )
{
	if (Vertex1 < 0 || Vertex1 >= Mesh.GetVertexCount() ||
		Vertex2 < 0 || Vertex2 >= Mesh.GetVertexCount())
	{
		throw std::invalid_argument( "vertex index" );
	}

	CVector3f const Position1 = Mesh.GetVertexPosition( Vertex1 );
	CVector3f const Position2 = Mesh.GetVertexPosition( Vertex2 );
	if (!Position1.Compare( Position2, KPositionTolerance ))
	{
		return false;
	}

	CVector3f const Normal1 = Mesh.GetVertexNormal( Vertex1 );
	CVector3f const Normal2 = Mesh.GetVertexNormal( Vertex2 );
	if (!Normal1.Compare( Normal2, KNormalTolerance ))
	{
		return false;
	}

	if (Mesh.HasDiffuse())
	{
		ARGB const Diffuse1 = Mesh.GetVertexDiffuse( Vertex1 );
		ARGB const Diffuse2 = Mesh.GetVertexDiffuse( Vertex2 );

		if (fabsf( static_cast<float32>( ((Diffuse1 >> 24) & 0xff) - ((Diffuse2 >> 24) & 0xff) ) ) > KColorTolerance) return false;
		if (fabsf( static_cast<float32>( ((Diffuse1 >> 16) & 0xff) - ((Diffuse2 >> 16) & 0xff) ) ) > KColorTolerance) return false;
		if (fabsf( static_cast<float32>( ((Diffuse1 >>  8) & 0xff) - ((Diffuse2 >>  8) & 0xff) ) ) > KColorTolerance) return false;
		if (fabsf( static_cast<float32>( ((Diffuse1 >>  0) & 0xff) - ((Diffuse2 >>  0) & 0xff) ) ) > KColorTolerance) return false;
	}

	for (int32 i = 0; i < Mesh.GetBetaCount(); ++i)
	{
		float32 Beta1 = Mesh.GetVertexBeta( Vertex1, i );
		float32 Beta2 = Mesh.GetVertexBeta( Vertex2, i );
		if (fabsf( Beta1 - Beta2 ) > KBetaTolerance)
		{
			return false;
		}
	}

	for (int32 i = 0; i < Mesh.GetMatrixCount(); ++i)
	{
		int32 Matrix1 = Mesh.GetVertexMatrix( Vertex1, i );
		int32 Matrix2 = Mesh.GetVertexMatrix( Vertex2, i );
		if (Matrix1 != Matrix2)
		{
			return false;
		}
	}

	for (int32 i = 0; i < Mesh.GetUVCount(); ++i)
	{
		CVector2f UV1 = Mesh.GetVertexUV( Vertex1, i );
		CVector2f UV2 = Mesh.GetVertexUV( Vertex2, i );
		if (!UV1.Compare( UV2, KUVTolerance ))
		{
			return false;
		}
	}

	return true;
}

void CMesh::Optimize( bool bMergeSimilarVertices )
{
	//Check if all vertices use a matrix index of zero. If so,
	//that matrix index isn't needed.
	int32 Matrix = m_MatrixCount - 1;
	for (; Matrix >= 0; --Matrix)
	{
		for (int32 i = 0; i < m_VertexCount; ++i)
		{
			if (GetVertexMatrix( i, Matrix ) != 0)
			{
				goto LDoneCheckingMatrices;
			}
		}
	}
LDoneCheckingMatrices:

	//Use only the minimum number of matrices needed.
	SetMatrixCount( Matrix + 1 );

	//Beta values in excess of the matrix count are pointless,
	//since the excess betas and all of the residual weight will be applied 
	//to matrix 0.
	if (m_BetaCount > m_MatrixCount)
	{
		SetBetaCount( m_MatrixCount );
	}

	if (bMergeSimilarVertices)
	{
		MergeSimilarVertices();
	}
}

uint32 WTCall DummyComputeAuxHash(void const*, int32) {return 0;}
bool WTCall DummyEqualAux(void const*, int32, int32) {return true;}
void CMesh::MergeSimilarVertices()
{
	std::vector<int32> VertexRemap;
	MergeSimilarVertices( NULL, DummyComputeAuxHash, DummyEqualAux, VertexRemap );
}

void CMesh::MergeSimilarVertices(
		void const* pAux,
		uint32 (WTCall* ComputeAuxHash)(void const*, int32),
		bool (WTCall* EqualAux)(void const*, int32, int32),
		std::vector<int32>& VertexRemap )
{
	//This function compares pairs of vertices, eliminating ones that are duplicates
	//or too "similar" to existing vertices. This takes N^2 time (slow), but this function 
	//is optimized to do as little N^2 work as possible.

	//Each vertex i in a part will be replaced with VertexRemap[i].
	VertexRemap.resize( m_VertexCount );
	for (int32 i = m_VertexCount - 1; i >= 0; --i)
	{
		VertexRemap[i] = i; //Initially, assume vertices are not duplicates.
	}

	//First, compute a hash value for all vertices
	std::vector<int32> Hashes( m_VertexCount );

	for (int32 i = 0; i < m_VertexCount; ++i)
	{
		int32 Hash = ComputeAuxHash( pAux, i );

		HashAdd( Hash, GetVertexPosition( i ).x, KPositionTolerance );
		HashAdd( Hash, GetVertexPosition( i ).y, KPositionTolerance );
		HashAdd( Hash, GetVertexPosition( i ).z, KPositionTolerance );

		HashAdd( Hash, GetVertexNormal( i ).x, KNormalTolerance );
		HashAdd( Hash, GetVertexNormal( i ).y, KNormalTolerance );
		HashAdd( Hash, GetVertexNormal( i ).z, KNormalTolerance );

		if (m_HasDiffuse)
		{
			HashAdd( Hash, static_cast<float32>( (GetVertexDiffuse( i ) >> 24) & 0xff ), KColorTolerance );
			HashAdd( Hash, static_cast<float32>( (GetVertexDiffuse( i ) >> 16) & 0xff ), KColorTolerance );
			HashAdd( Hash, static_cast<float32>( (GetVertexDiffuse( i ) >>  8) & 0xff ), KColorTolerance );
			HashAdd( Hash, static_cast<float32>( (GetVertexDiffuse( i ) >>  0) & 0xff ), KColorTolerance );
		}

		for (int32 j = 0; j < m_BetaCount; ++j)
		{
			float32 const Beta = GetVertexBeta( i, j );
			HashAdd( Hash, Beta, KBetaTolerance );
		}

		for (int32 j = 0; j < m_MatrixCount; ++j)
		{
			int32 const Matrix = GetVertexMatrix( i, j );
			HashAdd( Hash, static_cast<float32>( Matrix ), 1.0f );
		}

		for (int32 j = 0; j < m_UVCount; ++j)
		{
			CVector2f UV = GetVertexUV( i, j );
			HashAdd( Hash, UV.x, KUVTolerance );
			HashAdd( Hash, UV.y, KUVTolerance );
		}

		Hashes[i] = Hash;
	}

	{
		//Sort vertices indirectly by hash value. This results in contiguous sets of vertices
		//of the same hash value.
		std::vector<int32> Indirect( m_VertexCount );
		for (int32 i = m_VertexCount - 1; i >= 0; --i)
		{
			Indirect[i] = i;
		}

		std::sort( Indirect.begin(), Indirect.end(), MyPredicate( Hashes ) );

		//Find groups of vertices with the same hash value
		std::vector<int32> SortedVertices; //This vector will be used shortly
		SortedVertices.reserve( m_VertexCount );
		int32 First = 0;
		for (int32 i = 0; i <= m_VertexCount; ++i)
		{
			if (i != m_VertexCount && (i == First || Hashes[Indirect[i]] == Hashes[Indirect[i - 1]]))
			{
				continue;
			}

			//The group of vertices represented by indices from First up to i has the same hash value.
			//Sort these by vertex index. We want to have it so that if and only if vertex x is a duplicate 
			//of vertex y and y < x, then VertexRemap[x] = y.
			SortedVertices.clear();
			for (int32 j = First; j < i; ++j)
			{
				SortedVertices.push_back( Indirect[j] );
			}
			std::sort( SortedVertices.begin(), SortedVertices.end() );

			//Test each pair of vertices
			for (int32 j = static_cast<int32>( SortedVertices.size() ) - 1; j >= 0; --j)
			{
				for (int32 k = j - 1; k >= 0; --k)
				{
					//Note that k < j, so we want j to "point to" k if it is a duplicate
					if (VerticesEqual( *this, SortedVertices[j], SortedVertices[k] )
						&& EqualAux( pAux, SortedVertices[j], SortedVertices[k] ))
					{
						//Vertex j should be replaced with identical vertex k.
 						VertexRemap[SortedVertices[j]] = SortedVertices[k];
					}
				}
			}

			//Get ready for the next group
			First = i;
		}
	}

	int32 NumVertsDeleted = 0;
	for (int32 i = 0; i < static_cast<int32>( m_VertexCount ); ++i)
	{
		if (VertexRemap[i] == i)
		{
			VertexRemap[i] -= NumVertsDeleted; //Adjust for deleted prior vertices

			//Remove the vectors corresponding to the removed vertex indices
			if (NumVertsDeleted > 0)
			{
				CopyVertex( VertexRemap[i], i );
			}
		} else {
			++NumVertsDeleted; //All verts numbered above i have to be adjusted further

			//Get actual vertex number by remapping repeatedly. After the first remap we point
			//to a lesser vertex number (as ensured above) so only one additional remap is required
			//to get the actual original vertex index.
			VertexRemap[i] = VertexRemap[VertexRemap[i]];
		}
	}

	//Change part vertices
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart* pPart = m_hParts[i][j];
			for (int32 k = 0; k < pPart->GetVertexCount(); ++k)
			{
				pPart->SetVertex( k, VertexRemap[ pPart->GetVertex( k ) ] );
			}
		}
	}

	SetVertexCount( m_VertexCount - NumVertsDeleted );
}

#if 0
//Test code
//Split tri-strips into tri-strips having StripVerts vertices or fewer
void CMesh::SplitStrips( int32 StripVerts = 80 )
{
	for (int32 i = 0; i < ERenderCategory::Count; ++i)
	{
		for (size_t j = 0; j < m_hParts[i].size(); ++j)
		{
			CMeshPart* pPart = m_hParts[i][j];

			if (pPart->GetPartType() == CMeshPart::EType::TriStrip)
			{
				//Divide tri-strip into tri-strips of length StripVerts vertices or smaller
				if (pPart->GetVertexCount() > StripVerts)
				{
					HMeshPart hNewPart = new CMeshPart( this );
					hNewPart->SetFlags( pPart->GetFlags() );
					hNewPart->SetPartType( pPart->GetPartType() );
					hNewPart->SetMaterialID( pPart->GetMaterialID() );
					hNewPart->SetVertexCount(  pPart->GetVertexCount() - (StripVerts - 2) );
					for (int32 k = StripVerts - 2; k < pPart->GetVertexCount(); ++k)
					{
						hNewPart->SetVertex( k - (StripVerts - 2), pPart->GetVertex( k ) );
					}
					pPart->SetVertexCount( StripVerts );
					m_hParts[i].insert( m_hParts[i].begin() + (j + 1), hNewPart );
				}
			}
		}
	}

#if MESH_WHOLE_IB
	m_hIB = NULL;
#endif
}
#endif

} //namespace Graphics
} //namespace WT
