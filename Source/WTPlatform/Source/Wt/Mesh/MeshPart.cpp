/*
	MeshPart.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include <limits>
#include "d3d9types.h"
#include "WT/Mesh/MeshPart.h"
#include "WT/Mesh/Mesh.h"
#include "WT/System/IO/BitStream.h"

using namespace WT::System::IO;
using namespace WT::System::Serialization;
using namespace WT::Graphics;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS(CMeshPart)

//A pointer to index data. The indices may be 2 or 4 bytes each.
class IndexArray
{
public:
	IndexArray( void* pData, uint32 Size )
		: m_pData( static_cast<uint8*>( pData ) )
		, m_Size( Size )
	{
		assert( m_Size == sizeof( uint16 ) || 
				m_Size == sizeof( uint32 ) );
	};

	void Set( int32 Index, int32 Value )
	{
		switch (m_Size)
		{
			case sizeof( uint32 ):
				reinterpret_cast<uint32*>( m_pData )[Index] = static_cast<uint32>( Value );
				break;
			case sizeof( uint16 ):
				reinterpret_cast<uint16*>( m_pData )[Index] = static_cast<uint16>( Value );
				break;
			default:
				assert( false );
		}
	}

	void operator+=( int32 Offset )
	{
		m_pData += m_Size * Offset;
	}

private:
	uint8 *m_pData;
	int32 m_Size; //Size of indices in bytes (sizeof( uint16 ) or sizeof( uint32 ))
};

CMeshPart::EType::Type CMeshPart::GetPartType() const
{
    return m_PartType;
}

void CMeshPart::SetPartType( EType::Type PartType )
{
	if (PartType < 0 || PartType >= EType::Count)
    {
		throw std::out_of_range( "mesh part type" );
    }

	if (m_PartType != PartType)
	{
		SafeRelease( m_hIB );
	    m_PartType = PartType;
		ComputePrimType();
		ComputePrimCount();
	}
}

int32 CMeshPart::GetFlags() const
{
    return m_Flags;
}

void CMeshPart::SetFlags(int32 Flags)
{
	if (Flags < 0 || Flags > EFlags::Mask)
    {
		throw std::out_of_range( "mesh part flags" );
    }

	uint32 OldFlags = m_Flags;
	m_Flags = Flags;

	//Two-sided changed?
	if ((OldFlags ^ m_Flags) & EFlags::TwoSided)
	{
		ComputePrimCount();
	}
}

int32 CMeshPart::GetVertexCount() const
{
    return static_cast<int32>( m_Vertices.size() );
}

void CMeshPart::ClearVertices()
{
	SafeRelease( m_hIB );
    m_Vertices.clear();
	ComputePrimCount();
	m_bMinMaxDirty = true;
}

void CMeshPart::SetVertexCount( int32 VertexCount )
{
	if (VertexCount < 0)
	{
		throw std::domain_error( "vertex count" );
	}

    //The mesh part shouldn't refer to non-existent mesh vertices.
	//Instead of throwing an exception now, assume the mesh vertices will
	//be added later. TODO log this.
	//if (m_pMesh->GetVertexCount() <= 0)
	//{
	//	throw std::out_of_range( "vertex count" );
	//}

	if (VertexCount != static_cast<int32>( m_Vertices.size() ))
	{
		SafeRelease( m_hIB );
		m_Vertices.resize( VertexCount, 0 );
		ComputePrimCount();
		m_bMinMaxDirty = true;
	}
}

void CMeshPart::RemoveVertex( int32 Index )
{
	if (Index < 0 || static_cast<uint32>(Index) >= m_Vertices.size())
    {
		throw std::out_of_range( "mesh part vertex index" );
    }

	SafeRelease( m_hIB );
    m_Vertices.erase( m_Vertices.begin() + Index );
	ComputePrimCount();
	m_bMinMaxDirty = true;
}

int32 CMeshPart::GetVertex( int32 Index ) const
{
    if (Index < 0 || static_cast<uint32>(Index) >= m_Vertices.size())
    {
		throw std::out_of_range( "mesh part vertex index" );
    }

    return m_Vertices[Index];
}

void CMeshPart::SetVertex( int32 Index, int32 Vertex )
{
    assert( m_pMesh != NULL );

    if (Index < 0 || static_cast<uint32>( Index ) >= m_Vertices.size())
    {
		throw std::out_of_range( "mesh part vertex index" );
    }

    if (Vertex < 0 || Vertex >= m_pMesh->GetVertexCount())
    {
		throw std::out_of_range( "mesh vertex index" );
    }

	if (m_Vertices[Index] != static_cast<uint32>( Vertex ))
	{
		SafeRelease( m_hIB );
	    m_Vertices[Index] = Vertex;
		m_bMinMaxDirty = true;
	}
}

int32 CMeshPart::AddVertex( int32 Vertex )
{
	SafeRelease( m_hIB );
	m_Vertices.push_back( Vertex );
	ComputePrimCount();
	m_bMinMaxDirty = true;
	return static_cast<int32>( m_Vertices.size() );
}

void CMeshPart::InsertVertex( int32 Index, int32 Vertex )
{
	if (Index < 0 || static_cast<uint32>( Index ) > m_Vertices.size())
	{
		throw std::out_of_range("mesh part vertex index" );
	}

	if (static_cast<uint32>( Index ) == m_Vertices.size())
	{
		AddVertex( Vertex );
		return;
	}

	SafeRelease( m_hIB );
	m_Vertices.insert( m_Vertices.begin() + Index, Vertex );
	ComputePrimCount();
	m_bMinMaxDirty = true;
}

MeshMaterialID CMeshPart::GetMaterialID() const
{
    return m_MaterialID;
}

void CMeshPart::SetMaterialID( MeshMaterialID const& MaterialID )
{
    m_MaterialID = MaterialID;
}

int32 CMeshPart::GetUserBitCount() const
{
    return static_cast<int32>(m_UserBits.size());
}

bool CMeshPart::GetUserBit(int32 Index ) const
{
    if (Index < 0 || static_cast<uint32>(Index) >= m_UserBits.size())
    {
		throw std::out_of_range( "user bit index" );
    }

    return m_UserBits[Index];
}

void CMeshPart::SetUserBit(int32 Index, bool Value)
{
    if (Index < 0 || static_cast<uint32>(Index) >= m_UserBits.size())
    {
		throw std::out_of_range( "user bit index" );
    }

    m_UserBits[Index] = Value;
}

void CMeshPart::SetUserBitCount( int32 UserBitCount )
{
    assert( m_pMesh != NULL );

    if (UserBitCount < 0)
    {
		throw std::domain_error( "user bit count" );
    }

    if (UserBitCount != m_pMesh->GetUserBitCount())
    {
		throw std::invalid_argument( "user bit count" );
    }

    m_UserBits.resize( UserBitCount );
}

/// Get the total number of triangles in the part. Note: two-sided
/// triangles still count as single triangles.
int32 CMeshPart::ComputeTriangleCount() const
{
	int32 const NumPartVertices = static_cast<int32>( m_Vertices.size() );

	switch (m_PartType)
	{
		default:
			assert( false );
			break;

		case EType::TriList:
			return NumPartVertices / 3;

		case EType::TriStrip:
		case EType::TriFan:
		case EType::TriStripInverted:
			if (NumPartVertices > 2)
			{
				return NumPartVertices - 2;
			}
			break;

		case EType::QuadList:
			return NumPartVertices / 4;
	}

	return 0;
}

int32 CMeshPart::GetPrimCount() const
{
	return m_PrimCount;
}

int32 CMeshPart::GetIndexCount() const
{
	switch (m_PartType)
	{
		default:
			assert( 0 );
		case EType::PointList:
			return 0; //Point lists aren't supported by DrawIndexedPrimitive().

		case EType::LineList:
			return m_PrimCount << 1;
		
		case EType::LineStrip:
			return m_PrimCount + 1;

		case EType::TriList:
		case EType::QuadList:
			return m_PrimCount * 3; //Rendered using triangles on this platform

		case EType::TriStrip:
		case EType::TriStripInverted:
		case EType::TriFan:
			return m_PrimCount + 2;
	}
}

void CMeshPart::UpdateMinMaxAux()
{
	assert( m_bMinMaxDirty );

	m_bMinMaxDirty = false;

	if (m_Vertices.empty())
	{
		m_MinVertex = 0;
		m_MaxVertex = 0;
		return;
	}

	m_MinVertex = m_Vertices[0];
	m_MaxVertex = m_Vertices[0];

	for (size_t i = 1; i < m_Vertices.size(); ++i)
	{
		if (m_MinVertex > m_Vertices[i])
		{
			m_MinVertex = m_Vertices[i];
		} else if (m_MaxVertex < m_Vertices[i]) {
			m_MaxVertex = m_Vertices[i];
		}
	}
}

void CMeshPart::ComputePrimType()
{
	switch (m_PartType)
	{
	case EType::PointList:
		m_PrimType = D3DPT_POINTLIST; break;

	case EType::LineList:
		m_PrimType = D3DPT_LINELIST; break;

	case EType::LineStrip:
		m_PrimType = D3DPT_LINESTRIP; break;

	case EType::TriList:
	case EType::QuadList:
		m_PrimType = D3DPT_TRIANGLELIST; break;

	case EType::TriStrip:
	case EType::TriStripInverted:
		m_PrimType = D3DPT_TRIANGLESTRIP; break;

	case EType::TriFan:
		m_PrimType = D3DPT_TRIANGLEFAN; break;

	default:
		assert( false ); 
		m_PrimType = D3DPT_TRIANGLELIST;
		break;
	}
}

void CMeshPart::ComputePrimCount()
{
	m_PrimCount = 0;
	uint32 const VertexCount = static_cast<uint32>(m_Vertices.size());

	switch (m_PartType)
	{
	case EType::PointList: 
		m_PrimCount = VertexCount;
		break;

	case EType::LineList:
		m_PrimCount = VertexCount >> 1;
		break;

	case EType::LineStrip:
		if (VertexCount >= 2)
		{
			m_PrimCount = VertexCount - 1;
		}
		break;

	case EType::TriList:
		m_PrimCount = VertexCount / 3;
		if (m_Flags & EFlags::TwoSided)
		{
			m_PrimCount <<= 1;
		}
		break;

	case EType::TriStrip:
		if (VertexCount >= 3)
		{
			m_PrimCount = VertexCount - 2;
		}
		if (m_Flags & EFlags::TwoSided)
		{
			m_PrimCount = (m_PrimCount << 1) + 1;
		}
		break;

	case EType::TriStripInverted:
		if (VertexCount >= 3)
		{
			if (m_Flags & EFlags::TwoSided)
			{
				m_PrimCount = (VertexCount - 2) << 1;
			} else {
				m_PrimCount = (VertexCount - 2) | 1;
			}
		}
		break;

	case EType::TriFan:
		if (VertexCount >= 3)
		{
			m_PrimCount = VertexCount - 2;
		}
		if (m_Flags & EFlags::TwoSided)
		{
			m_PrimCount <<= 1;
		}
		break;

	case EType::QuadList:
		m_PrimCount = (VertexCount >> 1) & ~1;
		if (m_Flags & EFlags::TwoSided)
		{
			m_PrimCount <<= 1;
		}
		break;

	default:
		assert( false );
		break;
	}
}

void CMeshPart::LockVertices( uint32** pPtr )
{
	if (pPtr != NULL)
	{
		*pPtr = &m_Vertices[0];
	}
}

void CMeshPart::LockVertices( uint32 const** pPtr ) const
{
	if (pPtr != NULL)
	{
		*pPtr = &m_Vertices[0];
	}
}

void CMeshPart::UnlockVertices() const
{
	//TODO no changes really needed if read-only version of Lock called originally.
	SafeRelease( const_cast<CMeshPart*>( this )->m_hIB );
	const_cast<CMeshPart*>( this )->ComputePrimCount();
	const_cast<CMeshPart*>( this )->m_bMinMaxDirty = true;
}

void CMeshPart::OnVertexInserted( int32 Vertex )
{
	for (size_t j = 0; j < m_Vertices.size(); ++j)
	{
		if ( static_cast<int32>(m_Vertices[j]) >= Vertex )
		{	
			SafeRelease( m_hIB );
			m_bMinMaxDirty = true;
			++m_Vertices[j];
		}
	}
}

bool CMeshPart::OnVertexRemoved( int32 Index )
{
	for (size_t j = 0; j < m_Vertices.size(); ++j)
	{
		if (m_Vertices[j] == static_cast<uint32>( Index ) )
		{
			return false;
		}

		if ( static_cast<int32>(m_Vertices[j]) > Index )
		{
			SafeRelease( m_hIB );
			m_bMinMaxDirty = true;
			--m_Vertices[j];
		}
	}

	return true;
}

bool CMeshPart::OnVerticesTruncated( int32 VertexCount )
{
	for (size_t j = 0; j < m_Vertices.size(); ++j)
	{
		if ( static_cast<int32>(m_Vertices[j]) >= VertexCount )
		{
			return false;
		}
	}

	return true;
}

void CMeshPart::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, 
					   std::vector<HMaterial> const* pMaterials )
{
	assert( pDevice != NULL );

	AcquireDeviceResources();

	if (m_PrimCount == 0 ||
		(m_Flags & EFlags::Invisible))
	{
		return;
	}

	UpdateMinMax();
	assert( m_MinVertex <= m_MaxVertex );
	
#if !MESH_WHOLE_IB
	if (m_PartType != EType::PointList)
	{
		assert( m_hIB != NULL );
		pDevice->SetIndices( m_hIB );
	}
#endif

	//Set material
	CMaterial const* pMaterial = GetMaterial( pMaterials, m_MaterialID );
	CMaterial::SetCachedMaterial( pDevice, Context, pMaterial );

	pMaterial = CMaterial::GetCachedMaterial();
	int32 const PassCount = CMaterial::GetCachedPassCount();
	for (int32 i = 0; i < PassCount; ++i)
	{
		assert( pMaterial != NULL );
		pMaterial->BeginPass( pDevice, Context, i );

		if (m_PartType == EType::PointList)
		{
			//Point lists cannot use DrawIndexedPrimitives().
			for (int32 i = 0; i < m_PrimCount; ++i)
			{
				pDevice->DrawPrimitive( m_PrimType, m_Vertices[i], 1 );
			}
		} else {
#if MESH_WHOLE_IB
			pDevice->DrawIndexedPrimitive( 
				m_PrimType, 0, 

				//0, m_VertexCount,
				//Pass range of used vertices - hopefully an optimization
				m_MinVertex, m_MaxVertex - m_MinVertex + 1, 

				m_StartIndex,
				m_PrimCount );
#else
			pDevice->DrawIndexedPrimitive( 
				m_PrimType, 0, 

				//0, m_VertexCount,
				//Pass range of used vertices - hopefully an optimization
				m_MinVertex, m_MaxVertex - m_MinVertex + 1, 

				0,
				m_PrimCount );
#endif
		}

		pMaterial->EndPass();
	}
}

void CMeshPart::AddVertexNormalContribution_Line( int32 Index0, int32 Index1 )
{
	//To v0's normal sum, add (v0 - v1)/|v0 - v1|.
	//Apply method to each of the two vertices.

	CVector3f dN = m_pMesh->GetVertexPosition( Index0 ) - m_pMesh->GetVertexPosition( Index1 );
	dN.Normalize();

	//Add v0 to vertex 0's normal
	{
		CVector3f N = m_pMesh->GetVertexNormal( Index0 );
		N += dN;
		m_pMesh->SetVertexNormal( Index0, N );
	}

	//Subtract v0 from vertex 1's normal
	{
		CVector3f N = m_pMesh->GetVertexNormal( Index1 );
		N -= dN;
		m_pMesh->SetVertexNormal( Index0, N );
	}
}

void CMeshPart::AddVertexNormalContribution_Tri( int32 Index0, int32 Index1, int32 Index2 )
{
	//To v0's normal sum, add ((v1-v0) x (v2-v0)) / ((v1-v0)*(v1-v0)(v2-v0)*(v2-v0)).
	//This gives a weighting of sin(theta)/(|v1-v0||v2-v0|) to the triangle's normal,
	//where theta is the angle between v1 and v2 as seen from v0.
	//For a polyhedron inscribed in a sphere, this weighting results in the same normals as the
	//sphere's.
	//Apply method to each of the three vertices.

	int32 const VertexCount = 3;
	int32 Indices[3] = {Index0, Index1, Index2,};

	for (uint32 i = 0; i < VertexCount; ++i)
	{
		CVector3f const v1 = m_pMesh->GetVertexPosition( Indices[(i + 1) % VertexCount] ) -
							  m_pMesh->GetVertexPosition( Indices[i] );
		CVector3f const v2 = m_pMesh->GetVertexPosition( Indices[(i + 2) % VertexCount] ) -
							  m_pMesh->GetVertexPosition( Indices[i] );
		float32 const v1v1 = v1.LengthSquared();
		float32 const v2v2 = v2.LengthSquared();
		float32 const v1v1v2v2 = v1v1 * v2v2;

		if (v1v1v2v2 == 0.0f)
		{
			return; //Degenerate triangle - no contribution to normals
		}

		CVector3f dN = v1.CrossProduct( v2 ) / v1v1v2v2;
		m_pMesh->SetVertexNormal( Indices[i], m_pMesh->GetVertexNormal( Indices[i] ) + dN );
	}
}

void CMeshPart::Invert()
{
	switch (m_PartType)
	{
	case EType::PointList: 
	case EType::LineList:
	case EType::LineStrip:
		//Do nothing
		break;

	case EType::TriList:
		for (size_t i = 0; i + 1 < m_Vertices.size(); i += 3)
		{
			std::swap( m_Vertices[i], m_Vertices[i + 1] );
		}
		SafeRelease( m_hIB );
		break;

	case EType::TriStrip:
		m_PartType = EType::TriStripInverted;
		SafeRelease( m_hIB );
		ComputePrimCount();
		break;

	case EType::TriStripInverted:
		m_PartType = EType::TriStrip;
		SafeRelease( m_hIB );
		ComputePrimCount();
		break;

	case EType::TriFan:
		{
			int32 const Max = static_cast<int32>( (m_Vertices.size() - 1) >> 1 );
			for (int32 i = 1; i <= Max; ++i)
			{
				std::swap( m_Vertices[i], m_Vertices[m_Vertices.size() - i] );
			}
		}
		SafeRelease( m_hIB );
		break;

	case EType::QuadList:
		for (size_t i = 0; i + 2 < m_Vertices.size(); i += 4)
		{
			std::swap( m_Vertices[i], m_Vertices[i + 2] );
		}
		SafeRelease( m_hIB );
		break;

	default:
		assert( false );
	}
}

void CMeshPart::ReleaseDeviceResources()
{
	SafeRelease( m_hIB );
}

void CMeshPart::AcquireDeviceResourcesAux()
{
	assert( m_hIB == NULL );
	assert( m_PrimCount != 0 );
	assert( !(m_Flags & EFlags::Invisible) );

	LPDIRECT3DDEVICE9 pDevice = m_pMesh->GetDevice();
	if (pDevice == NULL)
	{
		throw std::runtime_error( "NULL device" );
	}

	D3DFORMAT IndexFormat = D3DFMT_INDEX16;
	uint32 IndexSize = sizeof( uint16 );
	
	UpdateMinMax();
	if (m_MaxVertex > std::numeric_limits<uint16>::max())
	{
		IndexFormat = D3DFMT_INDEX32;
		IndexSize = sizeof( uint32 );
	}

	int32 const IndexCount = GetIndexCount();
	if (IndexCount == 0)
	{
		return; //Not supported by DrawIndexedPrimitive(); no index buffer needed.
	}

	pDevice->CreateIndexBuffer( IndexCount * IndexSize, D3DUSAGE_WRITEONLY, IndexFormat, D3DPOOL_MANAGED, &m_hIB, NULL );
	if (m_hIB == NULL)
	{
		throw std::runtime_error( "CreateIndexBuffer failure" );
	}

	void* pData;
	HRESULT Result = m_hIB->Lock( 0, IndexCount * IndexSize, &pData, 0 );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "index buffer lock" );
	}

	FillIndexBuffer( pData, IndexSize );

	m_hIB->Unlock();
}

void CMeshPart::FillIndexBuffer( void* pData, int32 IndexSize )
{
	IndexArray I( pData, IndexSize );

	uint32 const VertexCount = static_cast<uint32>(m_Vertices.size());

	switch (m_PartType)
	{
		case EType::PointList:
			//Point lists aren't supported by DrawIndexedPrimitive().
			break;

		case EType::LineList:
			for (uint32 j = 1; j < VertexCount; j += 2)
			{
				I.Set( 0, m_Vertices[j - 1] );
				I.Set( 1, m_Vertices[j] );
				I += 2;
			}
			break;
		
		case EType::LineStrip:
			for (uint32 j = 0; j < VertexCount; ++j)
			{
				I.Set( j, m_Vertices[j] );
			}
			break;

		case EType::TriList:
			for (uint32 j = 2; j < VertexCount; j += 3)
			{
				I.Set( 0, m_Vertices[j - 2] );
				I.Set( 1, m_Vertices[j - 1] );
				I.Set( 2, m_Vertices[j] );
				I += 3;

				if (m_Flags & EFlags::TwoSided)
				{
					I.Set( 0, m_Vertices[j - 2] );
					I.Set( 1, m_Vertices[j] );
					I.Set( 2, m_Vertices[j - 1] );
					I += 3;
				}
			}
			break;

		case EType::TriStrip:
		case EType::TriStripInverted:
			if (m_Flags & EFlags::TwoSided)
			{
				int32 const IndexCount = m_PrimCount + 2;
				for (uint32 j = 0; j < VertexCount; ++j)
				{
					I.Set( j, m_Vertices[j] );

					if (j < VertexCount - 1)
					{
						assert( IndexCount - 1 - j >= 0 );
						I.Set( IndexCount - 1 - j, m_Vertices[j] );
					}
				}
			} else if (m_PartType == EType::TriStrip) {
				for (uint32 j = 0; j < VertexCount; ++j)
				{
					I.Set( j, m_Vertices[j] );
				}
			} else {
				//Inverted, single-sided tri-strip.
				//The vertices are rendered in opposite order, and if there are an even
				//number of vertices, then the first rendered (i.e. last) vertex must be repeated
				//to select the correct orientation.
				for (uint32 j = 0; j < VertexCount; ++j)
				{
					if (j == 0 && !(VertexCount & 1))
					{
						I.Set( 0, m_Vertices[VertexCount - 1 - j] );
						I += 1;
					}

					I.Set( j, m_Vertices[VertexCount - 1 - j] );
				}
			}
			break;

		case EType::TriFan:
			for (uint32 j = 0; j < VertexCount; ++j)
			{
				I.Set( j, m_Vertices[j] );

				if (m_Flags & EFlags::TwoSided)
				{
					if (j > 0 && j <= VertexCount - 2)
					{
						I.Set( (VertexCount << 1) - 2 - j, m_Vertices[j] );
					}
				}
			}
			break;

		case EType::QuadList:
			for (uint32 j = 3; j < VertexCount; j += 4)
			{
				I.Set( 0, m_Vertices[j - 3] );
				I.Set( 1, m_Vertices[j - 2] );
				I.Set( 2, m_Vertices[j - 1] );
				I.Set( 3, m_Vertices[j - 1] );
				I.Set( 4, m_Vertices[j] );
				I.Set( 5, m_Vertices[j - 3] );
				I += 6;

				if (m_Flags & EFlags::TwoSided)
				{
					I.Set( 0, m_Vertices[j - 3] );
					I.Set( 1, m_Vertices[j - 1] );
					I.Set( 2, m_Vertices[j - 2] );
					I.Set( 3, m_Vertices[j] );
					I.Set( 4, m_Vertices[j - 1] );
					I.Set( 5, m_Vertices[j - 3] );
					I += 6;
				}
			}
			break;

		default:
			assert( false ); //Unrecognized primitive type
	}
}

void CMeshPart::AddVertexNormalContribution()
{
	if (m_Flags & EFlags::TwoSided)
	{
		return; //Two-sided mesh parts have zero contribution to vertex normals
	}

	int32 const VertexCount = static_cast<int32>(m_Vertices.size());

	switch (m_PartType)
	{
		case EType::PointList:
			return;

		case EType::LineList:
			for (int32 j = 1; j < VertexCount; j += 2)
			{
				AddVertexNormalContribution_Line( m_Vertices[j - 1], m_Vertices[j] );
			}
			break;

		case EType::LineStrip:
			for (int32 j = 1; j < VertexCount; ++j)
			{
				AddVertexNormalContribution_Line( m_Vertices[j - 1], m_Vertices[j] );
			}
			break;

		case EType::TriList:
			for (int32 j = 2; j < VertexCount; j += 3)
			{
				AddVertexNormalContribution_Tri( m_Vertices[j - 2], m_Vertices[j - 1], m_Vertices[j] );
			}
			break;

		case EType::TriStrip:
		case EType::TriStripInverted:
			for (int32 j = 2; j < VertexCount; ++j)
			{
				int32 bOdd = j & 1;

				if (m_PartType == EType::TriStripInverted)
				{
					bOdd ^= 1;
				}

				AddVertexNormalContribution_Tri( m_Vertices[j - 2 + bOdd], m_Vertices[j - 1 - bOdd], m_Vertices[j] );
			}
			break;

		case EType::TriFan:
			for (int32 j = 2; j < VertexCount; ++j)
			{
				AddVertexNormalContribution_Tri( m_Vertices[0], m_Vertices[j - 1], m_Vertices[j] );
			}
			break;

		case EType::QuadList:
			for (int32 j = 3; j < VertexCount; j += 4)
			{
				AddVertexNormalContribution_Tri( m_Vertices[j - 3], m_Vertices[j - 2], m_Vertices[j - 1] );
				AddVertexNormalContribution_Tri( m_Vertices[j - 3], m_Vertices[j - 1], m_Vertices[j] );
			}
			break;

		default:
			assert( false ); //Unrecognized primitive type
	}
}

CMesh* CMeshPart::GetMesh()
{
	return m_pMesh;
}

CMesh const* CMeshPart::GetMesh() const
{
	return m_pMesh;
}

void CMeshPart::SetMesh( CMesh* pMesh )
{
	m_pMesh = pMesh;
}

CMeshPart::CMeshPart( CMesh* pMesh ) :
	m_MinVertex( 0 ),
	m_MaxVertex( 0 ),
	m_PrimCount( 0 ),
	m_PrimType( D3DPT_TRIANGLELIST ),
	m_bMinMaxDirty( false ),
	m_hIB( NULL )
{
	m_PartType = EType::TriList;
    m_Flags = 0;
    m_MaterialID = -1;

	if (pMesh == NULL)
	{
		throw std::invalid_argument( "NULL mesh pointer" ) ;
	}

    m_pMesh = pMesh;
    m_UserBits.resize( m_pMesh->GetUserBitCount() );
}

CMeshPart::CMeshPart( CMeshPart const& Part ) :
	m_MinVertex( Part.m_MinVertex ),
	m_MaxVertex( Part.m_MaxVertex ),
	m_PrimCount( Part.m_PrimCount ),
	m_PrimType( Part.m_PrimType ),
	m_bMinMaxDirty( Part.m_bMinMaxDirty ),
	m_PartType( Part.m_PartType ),
	m_Flags( Part.m_Flags ),
	m_MaterialID( Part.m_MaterialID ),
	m_pMesh( Part.m_pMesh ),
	m_UserBits( Part.m_UserBits ),
	m_hIB( NULL ),
	m_Vertices( Part.m_Vertices )
{
}

CMeshPart::~CMeshPart()
{
	ReleaseDeviceResources();
}

HMeshPart CMeshPart::Clone() const
{
	return new CMeshPart( *this );
}

void CMeshPart::AddValue( CSerializationInfo* sInfo ) const
{
    //First write the flags
	{
		CStream* pStream = sInfo->BeginStreamBlock( "Flags" );
		CBitStream Bits( pStream );
	    Bits.WriteIntegerLittleEndian( m_PartType, 4 );

		uint32 const FlagBits = 12;
		assert( EFlags::Mask < (1 << FlagBits) );
		Bits.WriteIntegerLittleEndian( m_Flags, FlagBits );
	    
		//Now write user bits
		for (size_t i = 0; i < m_UserBits.size(); ++i)
		{
			Bits.WriteBit( m_UserBits[i] );
		}
		Bits.FlushBits();
		sInfo->EndStreamBlock();
	}

	sInfo->AddValue( "MaterialID", m_MaterialID );

    uint32 VertexCount = static_cast<uint32>(m_Vertices.size());
    sInfo->AddValue( "VertexCount", VertexCount );

    int32 NumBits = 0;
    while ((1 << NumBits) < m_pMesh->GetVertexCount())
    {
        ++NumBits;
    }

	{
		CStream* pStream = sInfo->BeginStreamBlock( "Vertices" );
		CBitStream Bits( pStream );
		for (uint32 i = 0; i < m_Vertices.size(); ++i)
		{
			Bits.WriteIntegerLittleEndian( m_Vertices[i], NumBits );
		}
		Bits.FlushBits();
		sInfo->EndStreamBlock();
	}
}

void CMeshPart::GetValue( CSerializationInfo* sInfo )
{
    {
        //First, read the flags
		CStream* pStream = sInfo->BeginStreamBlock( "Flags" );
        CBitStream Bits( pStream );
        uint32 PartType;
        uint32 Flags;
        PartType = Bits.ReadIntegerLittleEndian( 4 );
        Flags = Bits.ReadIntegerLittleEndian( 12 );

        SetPartType( static_cast<EType::Type>( PartType ) );

        SetFlags( Flags );
        
        //Now read user bits
        for (uint32 i = 0; i < m_UserBits.size(); ++i)
        {
            m_UserBits[i] = Bits.ReadBit();
        }
 
        Bits.FlushBits();
		sInfo->EndStreamBlock();
    }

	MeshMaterialID MaterialID;
	uint32 VertexCount;

	sInfo->GetValue( "MaterialID", MaterialID );
	sInfo->GetValue( "VertexCount", VertexCount );

	m_MaterialID = MaterialID;
    SetVertexCount( VertexCount );
    
    int32 NumBits = 0;
    while ((1 << NumBits) < m_pMesh->GetVertexCount())
    {
        ++NumBits;
    }

    {
        //Read the mesh vertex indices used by the part
		CStream* pStream = sInfo->BeginStreamBlock( "Vertices" );
        CBitStream Bits( pStream );

        for (uint32 i = 0; i < VertexCount; ++i)
        {
            uint32 Index;
            Index = Bits.ReadIntegerLittleEndian( NumBits );
            m_Vertices[i] = Index;
        }

        Bits.FlushBits();
		sInfo->EndStreamBlock();
   }

	//Since we set m_Vertices manually above without calling SetVertex(),
	//we need to also set this dirty flag manually.
	m_bMinMaxDirty = true;
}

/// Compute an estimate of the serialized size in bytes.
int32 CMeshPart::EstimateSerializedSize() const
{
	//Assume that only a portion of the vertices are unique
	int32 const VertexCount = m_pMesh->GetVertexCount() >> 1;

	//Compute the number of bits needed to represent a vertex - this is also used during part
	//serialization.
	int32 NumBits = 0;
	while ((1 << NumBits) < VertexCount)
	{
		++NumBits;
	}

	return static_cast<int32>( 9 + (m_UserBits.size() + 7) / 8 + (m_Vertices.size() * NumBits + 7) / 8 );
}

} //namespace Graphics
} //namespace WildTangent
