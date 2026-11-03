/*
	MeshPart.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once

#include <vector>
#include "WT/System/Object.h"
#include "WT/System/SmartPointer.h"
#include "WT/Graphics/Material.h"

//Use a single index buffer for the whole mesh instead of per part?
//This isn't fully implemented currently - e.g. changing parts and vertices needs
//to mark the mesh IB as dirty. This define is for performance testing only.
//Currently turning this on appears to have the following effects:
//1. Slightly hurts CPU (e.g. 1% drop in CPU-bound scene)
//2. Slightly helps GPU (e.g. 2% speed up in GPU-bound scene).
//3. Makes manipulating mesh parts more cumbersome.
#define MESH_WHOLE_IB 0

namespace WT
{
namespace Graphics
{

//Forward declarations
class CMesh;

/// A material ID
typedef int32 MeshMaterialID;

/// A part of a mesh - for example, a list of points, a line strip, or a 
/// triangle list. A mesh part contains a list of vertex indices referring
/// to the vertices in the Mesh that the part belongs to. A mesh part also 
/// refers to a single material (which may be multi-textured).
///
/// Depending on the type of mesh part, some vertices may not be renderable.
/// For example, KTypeTriList requires a multiple of three vertices, so
/// if there are five vertices, the last two will not be renderable.
typedef System::SmartPointer<class CMeshPart> HMeshPart;
class CMeshPart : public System::CObject
{
public:
	WT_CLASS_NO_CREATOR( CMeshPart, System::CObject )

	// -------------------------------------------------------------
    // Serialization-related functions
    // -------------------------------------------------------------

    void AddValue( System::Serialization::CSerializationInfo* sInfo ) const;
	void GetValue( System::Serialization::CSerializationInfo* sInfo );

	/// Compute an estimate of the serialized size in bytes.
	int32 EstimateSerializedSize() const;

	// -------------------------------------------------------------
    // CMeshPart interface
    // -------------------------------------------------------------

	/// A mesh part type
	struct EType
	{
		enum Type
		{
			PointList = 0,        /// A list of points
			LineList = 1,         /// A list of line segments
			LineStrip = 2,        /// A line strip
			TriList = 3,          /// A list of triangles
			TriStrip = 4,         /// A triangle strip
			TriFan = 5,           /// A triangle fan
			QuadList = 6,         /// A list of quadrilaterals
			TriStripInverted = 7, /// A triangle strip of inverted sense

			Count /// Number of mesh part types (not itself a valid type)
		};
	};

	/// Mesh part flags
	struct EFlags
	{
		enum Type
		{
			TwoSided		= (1 << 0),		/// A two-sided part
			Invisible		= (1 << 1),		/// An invisible part
			Collidable		= (1 << 2),		/// A collidable part
			Translucent		= (1 << 3),		/// A translucent part - requires sorting unlike 
											/// opaque and 1-bit transparency parts.
			RenderFirst		= (1 << 4),
			RenderLast		= (1 << 5),
			Mask			= (1 << 6) - 1	/// Maximum valid combination of mesh part flags
		};
	};

    /// Get the type of part.
	EType::Type GetPartType() const;

    /// Set the type of part.
    void SetPartType( EType::Type PartType );

    /// Get the part flags. The result will range from 0 through KFlagMask.
    int32 GetFlags() const;

    /// Set the part flags. The value should range from 0 through MeshPart::KFlagMask.
    void SetFlags( int32 Flags );

    /// Get the number of vertices in the part.
    int32 GetVertexCount() const;

    /// Remove all of the part's vertices.
    void ClearVertices();

    /// Set the number of vertices in the part.
    void SetVertexCount( int32 VertexCount );

	/// Append a vertex to the mesh part.
	/// The index of the new part vertex is returned.
	int32 AddVertex( int32 Vertex );

    /// Remove a particular vertex from the part.
    void RemoveVertex( int32 Index );

	/// Insert a vertex into the mesh part. If Index is equal to the number of 
	/// part vertices, the new vertex is appended to the list.
	void InsertVertex( int32 Index, int32 Vertex );

	/// Get the mesh's vertex index corresponding to the specified part vertex.
    /// The result will range from 0 up to the number of vertices in the mesh.
    int32 GetVertex( int32 Index ) const;

    /// Set the mesh's vertex index used by the specified part vertex.
	/// Index must range from 0 up to the number of vertices in the part.
	/// Vertex must range from 0 up to the number of vertices in the mesh.
    void SetVertex( int32 Index, int32 Vertex );

    /// Get the ID of the material used by this part.
    MeshMaterialID GetMaterialID() const;

    /// Set the ID of the material used by this part.
    void SetMaterialID( MeshMaterialID const& MaterialID );

	/// Get the number of user bits per part.
    int32 GetUserBitCount() const;

    /// Get the value of a user bit.
    bool GetUserBit( int32 Index ) const;

    /// Set the value of a user bit.
    void SetUserBit( int32 Index, bool Value );

    /// Set the number of user bits. This should ONLY be called
    /// by the mesh when SetUserBitCount() is called on it.
    void SetUserBitCount( int32 UserBitCount );

	/// Get the total number of triangles in the part. Note: two-sided
	/// triangles still count as single triangles.
	int32 ComputeTriangleCount() const;

	/// Get the number of renderable primitives (points, line segments, or triangles).
	int32 GetPrimCount() const;

	/// Get the number of indices needed in an index buffer
	int32 GetIndexCount() const;

	//Fill a locked index buffer, given a pointer into it and the size of the index buffer elements.
	void FillIndexBuffer( void* pData, int32 IndexSize );

	/// Invert the orientation of this mesh part. As a result of this method,
	/// the type of the mesh part may change, or the vertex indices may be rearranged. 
	/// This method has an effect even for two-sided mesh parts.
	void Invert();

	/// Adapt to the insertion of a vertex at the specified position in the	Mesh's
	/// vertex list.
	void OnVertexInserted( int32 Vertex );

	/// Adapt to the removal of a vertex at the specified position in the Mesh's vertex
	/// list. If this mesh part contained the deleted vertex, then false should be returned
	/// to request this mesh part's destruction.
	bool OnVertexRemoved( int32 Index );

	/// Adapt to the truncation of the Mesh's vertex list starting at the specified index.
	/// If the mesh part contain the specified vertex, or any latter vertex, then false should
	/// be returned to request this mesh part's destruction.
	bool OnVerticesTruncated( int32 VertexCount );

	/// Render the mesh part to the specified graphics device.
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, 
		std::vector<HMaterial> const* pMaterials );

	/// Add the vertex normal contribution from this mesh part to the normal vectors of all
	/// relevant vertices. The normal vectors will be non-normalized during this process.
	void AddVertexNormalContribution();

	/// Lock the vertices in the mesh part.
	void LockVertices( uint32** pPtr );

	/// Perform a read-only lock of the vertices in this mesh part.
	void LockVertices( uint32 const** pPtr ) const;

	/// Unlock the previously locked vertices in this mesh part.
	void UnlockVertices() const;

	/// Get the mesh to which this part belongs.
	CMesh* GetMesh();

	/// Get the mesh to which this part belongs.
	CMesh const* GetMesh() const;

	/// Set the mesh to which this part belongs.
	void SetMesh( CMesh* pMesh );

	CMeshPart( CMesh* pMesh );
	CMeshPart( CMeshPart const& Part );
	virtual ~CMeshPart();

	HMeshPart Clone() const;

	__forceinline void AcquireDeviceResources()
	{
		//Index buffer already up-to-date or none needed?
		if (m_hIB != NULL ||
			m_PrimCount == 0 ||
			(m_Flags & EFlags::Invisible))
		{
			return;
		}
		
		AcquireDeviceResourcesAux();
	}

private:

	void AcquireDeviceResourcesAux();

	__forceinline void UpdateMinMax()
	{
		if (m_bMinMaxDirty)
		{
			UpdateMinMaxAux();
		}
	}

	void UpdateMinMaxAux();
	void ComputePrimType();
	void ComputePrimCount();

	void ReleaseDeviceResources();

	//Functions helping AddVertexNormalContribution.
	void AddVertexNormalContribution_Line( int32 Index0, int32 Index1 );
	void AddVertexNormalContribution_Tri( int32 Index0, int32 Index1, int32 Index2 );

	CMesh* m_pMesh;
    EType::Type m_PartType;
    int32 m_Flags;
	
	std::vector<uint32> m_Vertices; //Indices into mesh vertices

	std::vector<bool> m_UserBits;
    MeshMaterialID m_MaterialID;

	//Rendering-related member variables
	uint32				m_MinVertex;
	uint32				m_MaxVertex;
	int32				m_PrimCount; //Number of primitives (points, line segments, or tris)
	D3DPRIMITIVETYPE	m_PrimType;
	LPDIRECT3DINDEXBUFFER9	m_hIB;
	bool				m_bMinMaxDirty : 1;

#if MESH_WHOLE_IB
	int32				m_StartIndex;

	friend CMesh;
#endif
};

// Functions

inline CMaterial* GetMaterial( std::vector<HMaterial> const* pMaterials, int32 MaterialID )
{
	if (pMaterials == NULL)
	{
		return NULL;
	}
	
	if (MaterialID < 0 || MaterialID >= static_cast<MeshMaterialID>( pMaterials->size() ))
	{
		return NULL;
	}

	return pMaterials->operator[]( MaterialID );
}

} //namespace Graphics

template <>
inline void System::Serialization::CSerializationInfo::AddValue(const wstring& name, Graphics::CMeshPart const& Part)
{
	AddContainerBegin(name);
	Part.AddValue( this );
	AddContainerEnd();
}

template <>
inline void System::Serialization::CSerializationInfo::GetValue(const wstring& name, Graphics::CMeshPart& Part)
{
	GetContainerBegin(name);
	Part.GetValue( this );
	GetContainerEnd();
}

} //namespace WT
