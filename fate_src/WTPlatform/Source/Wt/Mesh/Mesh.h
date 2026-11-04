/***************************************************************************************
 *
 * Mesh.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          1/6/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/SmartPointer.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/File.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Math/Math.h"
#include "WT/Math/MathSerialization.h"
#include "WT/Math/LinearAlgebra/Vector2f.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Mesh/MeshPart.h"
#include "WT/Graphics/IRenderable.h"

namespace WT
{
namespace Graphics
{

using namespace Math;

typedef uint32 ARGB; //An A8R8G8B8 format color
//TODO use D3DCOLOR

class CMeshVtxPropCol;

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/// A renderable and serializeable mesh, consisting of vertices and mesh parts (typically 
/// tri-strips or tri-lists). All vertices are of the same format and are described by a 
/// position, normal vector, diffuse color, and a variable number of beta (bone weight) values, 
/// vertex blending matrix indices, and 2D-texture coordinates. Each mesh part may refer to a 
/// material and contains a list of vertex indices. Mesh parts logically contain a collection of 
/// either points, line segments, or triangles.
///
/// A Mesh object renders efficiently when used with a single graphics device. If hardware
/// vertex blending is possible and is needed by the mesh (i.e. it has betas and matrix indices),
/// then hardware vertex blending will be performed and it is expected that you set all
/// the needed world matrices in the D3D device before calling Render(). If hardware vertex blending
/// is not used, then you are responsible for setting the 0th world matrix only.
///
/// Software vertex blending is not performed, but you may use SetRenderGeoData() to supply your 
/// own transformed positions and normals. GetGeoData() may also be used to perform bone-space vertex 
/// position and normal manipulation (e.g. for morph targets).
///
/// It is suggested to set the D3DRS_NORMALIZENORMALS state in the device to TRUE if you 
/// want to render a Mesh that has non-normalized normals (e.g. due to vertex blending).
///
typedef System::SmartPointer<class CMesh> HMesh;
class CMesh : public System::CObject, public System::Serialization::ISerializable, public IDeviceObject
{
public:
	WT_BEGIN_CLASS( CMesh, System::CObject );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( IDeviceObject );
	WT_END_CLASS();

	/// A collection of parameters used for mesh serialization.
	/// To serialize with specific parameters, use the following code:
	///
	/// CMesh::SerializationParameters Params; //Create a Params object initialized to default parameters
	/// Params; //Set Params members here
	/// CSerializationContext SContext; //Create a serialization context
	///	SContext.GetProperties()->SetValue( "MeshParameters", reinterpret_cast<uint64>( &Params ) ); //Use the parameters
	struct SerializationParameters
	{
		SerializationParameters();

		uint32	VertexCacheSize;			 /// Uint32 specifying vertex cache size to optimize for when generating tri-strips
		uint8	FloatPrecision;				 /// Uint8 (0 - 24) specifying number of bits of precision in floating point values; default 8
		uint8	FloatPrecisionPositions;	 /// Uint8 override for position properties; default 16
		uint8	FloatPrecisionNormals;		 /// Uint8 override for position properties; default 10
		uint8	FloatPrecisionUVs;			 /// Uint8 override for UV properties; default 16
		bool	bEliminateDefaultProperties; /// bool
		bool	bUseHuffmanCompression;		 /// bool enabling Huffman compression (lossless) on integer data
		bool	bUseIndexedData;			 /// bool enabling creation of indexed floating-point data
		bool	bBuildTriStrips;			 /// bool enabling tri-stripping
		bool	bUseVQ;						 /// bool enabling vector quantization (lossy) on floating-point data; default true
		bool	bUseVQPositions;			 /// bool override for position properties; default false
		bool	bUseVQNormals;				 /// bool override for position properties; default false
		bool	bUseVQUVs;					 /// bool override for UV properties; default false
	};

	// -------------------------------------------------------------
    // ISerializable methods
    // -------------------------------------------------------------
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void FinalizeDeserialization( System::Serialization::CSerializationInfo *sInfo, System::Serialization::CSerializationContext* sContext );

	static uint32 GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}

	// -------------------------------------------------------------
    // IDeviceObject methods
    // -------------------------------------------------------------
	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	// -------------------------------------------------------------
    // CMesh serialization-related methods
    // -------------------------------------------------------------

	/// Compute an estimate of the serialized size in bytes.
	int32 EstimateSerializedSize( SerializationParameters* pParameters ) const;

	// -------------------------------------------------------------
    // CMesh interface
    // -------------------------------------------------------------

	//
	// Constants & types
	//

	static uint32 const KMaxTextureCount = 8;

	static uint32 const KMaxBetaCount = 3;
	static uint32 const KMaxMatrixCount = 4;

	typedef std::vector<HMeshPart> Parts;

	struct GeometryData
	{
		Math::CVector3f m_Position;
		Math::CVector3f m_Normal;

		bool operator==( GeometryData const& Data ) const
		{
			return 
				m_Position == Data.m_Position &&
				m_Normal == Data.m_Normal;
		}
	};

	//
	// Named constructors
	//

	/// Creates a cylinder or cone mesh.
	static inline HMesh WTCall CreateCylinder( LPDIRECT3DDEVICE9 pDevice,
		float32 Height, float32 BottomRadius, float32 TopRadius, 
		int32 VerticalDivisionCount, int32 AnnularDivisionCount );

	/// Creates a sphere mesh with one set of texture coordinates and a Plate carrée projection.
	/// The sphere contains a semicircular seam including the points (0,-Radius,0), (0,Radius,0), 
	/// and (0,0,-Radius). The seam includes vertex indices 0 (the bottom pole), VertexCount - 1 
	/// (the top pole), and 1 + i * (LongitudinalDivisionCount + 1) and (i + 1) * 
	/// (LongitudinalDivisionCount + 1), where i ranges from 0 through 
	/// (LattitudinalDivisionCount - 2).
	static inline HMesh WTCall CreateSphere( LPDIRECT3DDEVICE9 pDevice, float32 Radius, 
		int32 LattitudinalDivisionCount, int32 LongitudinalDivisionCount );

	/// Creates a smooth sphere mesh. LevelCount must not exceed 5.
	static inline HMesh WTCall CreateSmoothSphere( LPDIRECT3DDEVICE9 pDevice, float32 Radius, 
		int32 LevelCount );

	/// Creates a box mesh with one set of texture coordinates.
	static inline HMesh WTCall CreateBox( LPDIRECT3DDEVICE9 pDevice,
			float32 Width, float32 Height, float32 Depth,
			int32 XDivisionCount, int32 YDivisionCount, int32 ZDivisionCount );

	/// Creates a rectangle mesh with one set of texture coordinates. If both dimensions are positive, then
	/// the normal vector of the rectangle is in the positive Z-direction.
	static inline HMesh WTCall CreateRect( LPDIRECT3DDEVICE9 pDevice, float32 Width, float32 Height, 
			int32 XDivisionCount, int32 YDivisionCount );

	Parts& GetParts( ERenderCategory::Type RenderCategory = ERenderCategory::Opaque );

	Parts const& GetParts( ERenderCategory::Type RenderCategory = ERenderCategory::Opaque ) const;

	/// Tells whether translucent primitives are sorted. The default is true.
	bool GetSortTranslucent() const;

	/// Sets whether translucent primitives are sorted. The default is true. Setting this
	/// to false will result in faster rendering, but could produce more rendering anomalies.
	void SetSortTranslucent( bool bSortTranslucent );

	/// Tells whether the mesh is static. This is a hint that affects optimizations only; a static 
	/// mesh can still change.
	bool GetStatic() const;

	/// Sets whether the mesh is static. The default is true. This is a hint that affects optimizatons 
	/// only; a static mesh can still change.
	void SetStatic( bool Static );

	/// Clears all vertices, diffuse color, UV's, betas, matrix indices, mesh parts, and materials.
	void Clear();

	/// Get the number of vertices.
	int32 GetVertexCount() const;

	/// Get the number of per-part user bits.
	int32 GetUserBitCount() const;

	/// Compute the total number of renderable primitives (points, line
	/// segments, or triangles) in the mesh. Invisible mesh parts are excluded.
	int32 ComputeRenderablePrimCount() const;

	/// Get the total number of triangles in the mesh. Two-sided triangles are counted
	/// as a single triangle.
	int32 ComputeTriangleCount() const;

	/// Get the number of per-vertex 2D textures coordinate sets.
	int32 GetUVCount() const;

	/// Get the number of beta (bone weighting) values per vertex.
	int32 GetBetaCount() const;

	/// Get the number of matrices per vertices. Used for indexed vertex blending.
	int32 GetMatrixCount() const;

	/// Get the positon of the specified vertex.
	Math::CVector3f GetVertexPosition( int32 Index ) const;

	/// Get the normal of the specfied vertex.
	Math::CVector3f GetVertexNormal( int32 Index ) const;

	/// Get the diffuse color of the specified vertex.
	ARGB GetVertexDiffuse( int32 Index ) const;

	/// Get the texture coordinate of the specified vertex
	/// for the specified texture stage.
	Math::CVector2f GetVertexUV( int32 Index, int32 StageIndex ) const;

	/// Get the beta value for the specified vertex and blending index.
	float32 GetVertexBeta( int32 Index, int32 BlendIndex ) const;

	/// Get the matrix for the specified vertex and matrix index. 0 is returned on failure.
	int32 GetVertexMatrix( int32 Index, int32 MatrixIndex ) const;

	/// Clears all vertices as well as the mesh parts depending on them. Unlike
	/// Clear(), this retains the current vertex format.
	void ClearVertices();

	/// Add a new vertex. The index of the new vertex is returned.
	int32 AddVertex();

	/// Insert a new vertex at the specified position in the vertex
	/// list. Any vertex at or following the specified index will
	/// be shifted forward one index. If the specified index value is 
	/// equal to the number of vertices, then the new vertex is simply added.
	void InsertVertex( int32 Index );

	/// Remove the vertex with the specified index. Mesh parts depending on this
	/// index will be destroyed.
	void RemoveVertex( int32 Index );

	/// Set the number of vertices. If the number of vertices is decreased,
	/// then certain vertices, as well as the mesh parts depending on them, will be 
	/// destroyed. If the number of vertices is increased, additional vertices will
	/// be appended to the end of the vertex list.
	void SetVertexCount( int32 VertexCount );

	/// Set the number of per-part user bits.
	void SetUserBitCount( int32 UserBitCount );

	/// Test whether the mesh has a per-vertex diffuse color component
	bool HasDiffuse() const;

	/// Set whether the mesh has a per-vertex diffuse color component
	void SetHasDiffuse( bool HasDiffuse );

	/// Remove all texture coordinate sets.
	void ClearUVs();

	/// Add a per-vertex 2D texture coordinate set. On success, the index of the 
	/// new texture coordinate set is returned. On failure, -1 is returned.
	int32 AddUV();

	/// Insert a new texture coordinate set at the specified position in the texture 
	/// coordinates list. Any texture coordinate set at or following the specified index will
	/// be shifted forward one index. If the specified index value is equal to the number 
	/// of texture coordinate sets, then the new texture coordinate set is simply added.
	bool InsertUV( int32 Index );

	/// Remove the texture coordinate set with the specified index.
	void RemoveUV( int32 Index );

	/// Set the number of per-vertex 2D textures coordinate sets.
	bool SetUVCount( int32 UVCount );

	// Remove all beta sets.
	void ClearBetas();

	/// Add a per-vertex beta value. On success, the index of the new beta value is returned.
	/// On failure, -1 is returned.
	int32 AddBeta();

	/// Insert a new beta value at the specified position in the beta sets. Any beta
	/// set at or following the specified index will be shifted forward one index.
	/// If the specified index value is equal to the number of beta sets, then the new
	/// beta set is simply added.
	bool InsertBeta( int32 Index );

	/// Remove the beta set with the specified index.
	void RemoveBeta( int32 Index );

	/// Set the number of per-vertex beta values.
	bool SetBetaCount( int32 BetaCount );

	// Remove all matrices.
	void ClearMatrices();

	/// Add a per-vertex matrix. On success, the index of the new matrix is returned.
	/// On failure, -1 is returned.
	int32 AddMatrix();

	/// Insert a matrix at the specified position in the matrix list. Any matrices
	/// at or following the specified index will be shifted forward one index.
	/// If the specified index value is equal to the number of matrices, then the new
	/// matrix is simply added.
	bool InsertMatrix( int32 Index );

	/// Remove the matrix with the specified index.
	void RemoveMatrix( int32 Index );

	/// Set the number of per-vertex matrices.
	bool SetMatrixCount( int32 MatrixCount );

	/// Set the positon of the specified vertex.
	void SetVertexPosition( int32 Index, Math::CVector3f const& Position );

	/// Set the positon of the specified vertex.
	void SetVertexPosition( int32 Index, float32 PositionX, float32 PositionY, float32 PositionZ );

	/// Set the normal of the specfied vertex.
	void SetVertexNormal( int32 Index, Math::CVector3f const& Normal );

	/// Set the normal of the specfied vertex.
	void SetVertexNormal( int32 Index, float32 NormalX, float32 NormalY, float32 NormalZ );

	/// Set the diffuse color of the specified vertex.
	void SetVertexDiffuse( int32 Index, ARGB Diffuse );

	/// Set the diffuse color of all current vertices.
	void SetDiffuse( ARGB Diffuse );

	/// Set the texture coordinate of the specified vertex for the specified texture stage.
	void SetVertexUV( int32 Index, int32 StageIndex, Math::CVector2f const& UV );

	/// Set the texture coordinate of the specified vertex for the specified texture stage.
	void SetVertexUV( int32 Index, int32 StageIndex, float32 U, float32 V );

	/// Set the beta value of the specified vertex for the specified blend index.
	void SetVertexBeta( int32 Index, int32 BlendIndex, float32 Beta );

	/// Set the vertex matrix index of the specified vertex for the specified matrix index.
	void SetVertexMatrix( int32 VertexIndex, int32 MatrixIndex, int32 VertexMatrixIndex );

	/// Copy the data from the vertex specified by SourceIndex to the vertex specified by DestIndex.
	void CopyVertex( int32 DestIndex, int32 SourceIndex );

	/// Add another mesh's vertices and mesh parts into this one.
	void Add( CMesh const& mesh );

	/// Compute vertex normals based on current mesh parts and vertex positions.
	/// This function is called automatically when a mesh is loaded and it is found
	/// to lack valid normal data.
	void ComputeVertexNormals();

	//
	// Transformation functions
	//

	/// Transform the vertex positions and normals according to the specified matrix. 
	/// Normal vectors may and mesh part orientation may be updated. Note that it's
	/// assumed that the matrix satisfies _14 == _24 == _34 == 0 and _44 == 1.
	void Transform( Math::CMatrix4f const& Transform );

	/// Scale the mesh uniformly. Vertex positions will be scaled and, if Scale is negative,
	/// normals and mesh part orientations will be updated.
	void Scale( float32 Scale );

	/// Non-uniformly scale the vertex positions of the mesh according to a diagonal 
	/// matrix (represented as a vector). Normal vectors and mesh part orientations may be updated.
	void Scale( Math::CVector3f const& Scale );

	/// Non-uniformly scale the vertex positions of the mesh according to the three specified 
	/// scale factors. Normal vectors amd mesh part orientations may be updated.
	void Scale( float32 ScaleX, float32 ScaleY, float32 ScaleZ );

	/// Translate the vertex positions of the mesh by the specified translation vector.
	void Translate( Math::CVector3f const& Translation );

	/// Translate the vertex positions of the mesh by the specified translation vector.
	void Translate( float32 TranslateX, float32 TranslateY, float32 TranslateZ );

	//
	// Optimization functions
	//

	//Perform in-place optimizations on the mesh that increase performance when rendering
	//or detecting intersections. If bMergeSimilarVertices is true, then similar vertices
	//are merged together. Using bMergeSimilarVertices is dangerous for objects such as CActor
	//that have external data (morph targets) referring to vertex indices.
	void Optimize( bool bMergeSimilarVertices = true );

	//Merge similar vertices. This is a simplified version of the function below
	//which is appropriate to use when user-defined data external to the CMesh
	//objects affects the determination of which vertices are similar.
	void MergeSimilarVertices();

	//Merge similar vertices. pAux is passed in to the functions specified in latter
	//arguments; it is suggested to use pAux to locate auxilliary data.
	//ComputeAuxHash computes a hash value for any auxiliary data to be taken into
	//consideration. (A function returning zero is satisfactory if there is no such data.)
	//EqualAux returns whether the auxiliary data for two vertices is equal. (The function
	//should return true if there is no such data).
	//VertexRemap is set to results showing how the original vertex indices are mapped to 
	//new vertex indices.
	void MergeSimilarVertices(
		void const *pAux,
		uint32 (WTCall* ComputeAuxHash)(void const*, int32),
		bool (WTCall* EqualAux)(void const*, int32, int32),
		std::vector<int32>& VertexRemap );

	//
	// Advanced functions
	// These give you more powerful control over Mesh internal data, but must be 
	// used carefully.
	//

	/// Get the D3D vertex buffer format.
	DWORD GetVertexFormat() const;

	/// Get the size in bytes of each vertex in the D3D vertex buffer.
	uint32 GetVertexSize() const;

	/// Get the vertex declaration.
	LPDIRECT3DVERTEXDECLARATION9 GetVertexDeclaration();

	/// Get the internal vertex buffer. It may be NULL if the mesh has not yet been rendered
	/// with a LPDIRECT3DDEVICE9. Writing to the vertex buffer is not suggested as changes may
	/// be overwritten by later calls to CMesh methods.
	LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer();

	/// Renders the object from the perspective of the given camera.
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, 
		std::vector<HMaterial> const* pMaterials );

	//Check if software blending is appropriate for the mesh on the current hardware.
	//If true, skinned vertex locations should be passed into SetRenderGeoData(). 
	bool UsesSoftwareBlending() const
	{
		return m_bUseSoftwareBlending;
	}

	//Check if software blending is appropriate for the mesh on the current hardware.
	//If true, SetRenderGeoData() should be called and world matrices should be set
	//on the device before calling Render().
	bool UsesHardwareBlending() const
	{
		return m_bUseHardwareBlending;
	}

	explicit CMesh( LPDIRECT3DDEVICE9 pDevice = NULL );
	CMesh( CMesh const& Mesh );
	virtual ~CMesh();

	HMesh Clone() const;

	LPDIRECT3DDEVICE9 GetDevice() const {return m_pDevice;}
	
	__forceinline void SetDevice( LPDIRECT3DDEVICE9 pDevice )
	{
		if (m_pDevice != pDevice)
		{
			SetDeviceAux( pDevice );
		}
	}

	//Get the geometry data of the mesh. Unaligned SSE access to elements of GetGeoData() is safe.
	GeometryData const* GetGeoData() const
	{
		return &m_GeoData[0];
	}

	//Get the geometry data to be rendered by the mesh.
	//This could be set to a user-defined buffer containing 
	//animated or deformed positions; if NULL then the
	//mesh's internal geometry data is used.
	GeometryData const* GetRenderGeoData() const
	{
		return m_pRenderGeoData;
	}

	//Set the geometry data rendered by the mesh and tell
	//the mesh to update its VB. Use NULL to use the mesh's
	//internal geometry data. GetGeoData() should not be
	//passed in as it may change unexpectedly.
	void SetRenderGeoData( GeometryData const* pRenderGeoData )
	{
		if (m_pRenderGeoData == pRenderGeoData &&
			m_pRenderGeoData == NULL)
		{
			return;
		}

		m_pRenderGeoData = pRenderGeoData;
		m_bDirtyVB = true;
	}

	void AcquireDeviceResources();

private:

	void SetDeviceAux( LPDIRECT3DDEVICE9 pDevice );

	void AcquireDeviceResourcesMesh();
	void ReleaseDeviceResources();

	void UpdateBlendingMode();

	//
	// Functions
	//

	void UpdateVertexFormat();
	void UpdateVertexDeclaration();
	
	//Change vertex data.
	void RemoveVertexBytes( uint32 StartByte, uint32 EndByte );
	void InsertVertexBytes( uint32 StartByte, uint32 EndByte );
	void ZeroVertexBytes( uint32 StartByte, uint32 EndByte );

	inline int32 GetBetaBytes() const {return sizeof( float32 ) * m_BetaCount;}
	inline int32 GetMatrixBytes() const {return (m_MatrixCount + 3) & ~3;}
	inline int32 GetBetaMatrixBytes() const {return GetBetaBytes() + GetMatrixBytes();}
	inline int32 GetDiffuseBytes() const {return m_HasDiffuse ? sizeof( ARGB ) : 0;}
	inline int32 GetUVByteOffset() const {return GetBetaBytes() + GetMatrixBytes() + GetDiffuseBytes();}

	inline uint8* GetVertexPtr( int32 Index ) {return &m_VertexData[0] + Index * m_VertexSize;}
	inline uint8 const* GetVertexPtr( int32 Index ) const {return &m_VertexData[0] + Index * m_VertexSize;}
    
	void InvertParts();

	void FillVB( uint8 pData[] ) const;
	void FillVBGeometry( uint8 pData[] ) const;

private:
	//
	//Methods
	//

	void RenderTranslucentWithSorting( LPDIRECT3DDEVICE9 pDevice,
	                                   CRenderContext const& Context, Parts const& parts,
	                                   std::vector<HMaterial> const* pMaterials );

private:
	//
	//Member data
	//

	int32				m_UVCount;
	int32				m_BetaCount;
	int32				m_MatrixCount;

	uint32				m_VertexSize;
	int32				m_VertexCount;
	int32				m_UserBitCount;

	//Positions and normals
	std::vector<GeometryData>	m_GeoData;
	GeometryData const*			m_pRenderGeoData;

	//Other per-vertex data such as beta weights, matrix indices, diffuse colors, and all texture coordinates
	std::vector<uint8>			m_VertexData;

	Parts						m_hParts[ERenderCategory::Count];
	
	//Graphics device to render to. (Currently a CMesh cannot be rendered 
	//by multiple devices.)
	LPDIRECT3DDEVICE9		m_pDevice;
	LPDIRECT3DVERTEXBUFFER9	m_hVB;
	LPDIRECT3DVERTEXDECLARATION9 m_hVDecl;

	DWORD					m_VBVertexFormat;
	uint32					m_VBVertexSize;

	bool				m_HasDiffuse				: 1; //Does the mesh have a per-vertex diffuse color component?
	bool				m_bSortTranslucent			: 1;
	bool				m_bCanHardwareBlend			: 1;
	bool				m_bUseSoftwareBlending		: 1;
	bool				m_bUseHardwareBlending		: 1;
	bool				m_bDirtyVB					: 1; //VB is dirty
	bool				m_bUsingTranslucentIB		: 1; //True if using shared index buffers for translucent sorting
	bool				m_Static					: 1; //Is geometry "static"? (used for optimizations only)

#if MESH_WHOLE_IB
	LPDIRECT3DINDEXBUFFER9  m_hIB;
	friend class CMeshPart;
#endif
	
	//Temporary storage used to pass data from the first stage of deserialization
	//to the second stage of deserialization.
	std::vector<CMeshVtxPropCol> PropCols;

	friend class CActor;
};

// -------------------------------------------------------------------------------------
// Implementation
// -------------------------------------------------------------------------------------

//The MeshHelper interface helps you use Mesh but isn't an essential piece of its abstraction.
class CMeshHelper
{
public:
	static uint32 const KMaxSmoothSphereLevelCount = 5;

	/// Sets the mesh to be a cylinder or cone.
	static void WTCall SetCylinder( CMesh& M, float32 Height, float32 BottomRadius, float32 TopRadius,
		int32 VerticalDivisionCount, int32 AnnularDivisionCount );

	/// Sets the mesh to be a sphere.
	static void WTCall SetSphere( CMesh& M, float32 Radius, 
		int32 LattitudinalDivisionCount, int32 LongitudinalDivisionCount );

	/// Sets the mesh to be a sphere with more uniformly distributed vertices.
	/// LevelCount controls the complexity of the sphere. The minimum allowed value
	/// of LevelCount, 0, results in 20 triangles. The maximum allowed value of LevelCount, 5,
	/// yields 61,440 triangles.
	static void WTCall SetSmoothSphere( CMesh& M, float32 Radius, int32 LevelCount );

	/// Sets the mesh to be a box (a.k.a. cuboid) with one set of texture coordinates.
	static void WTCall SetBox( CMesh& M, float32 Width, float32 Height, float32 Depth,
		int32 XDivisionCount, int32 YDivisionCount, int32 ZDivisionCount );

	/// Sets the mesh to be a rectangle with one set of texture coordinates. Assuming both
	/// dimensions are positive, the normal vector of the rectangle is in the positive Z-direction.
	static void WTCall SetRect( CMesh& M, float32 Width, float32 Height, 
		int32 XDivisionCount , int32 YDivisionCount );

	//Add a parallelogram to the mesh part
	static void WTCall AddParallelogram( CMesh& M, Math::CVector3f const& Origin,
		Math::CVector3f const& Delta1, 
		Math::CVector3f const& Delta2, 
		int32 Div1Count, int32 Div2Count );

	static void WTCall SetTetrahedron( CMesh& M );
	static void WTCall SetIcosahedron( CMesh& M );
	static void WTCall SphereSubdivision( CMesh& M ); //Subdivide triangles into 4 smaller triangles. Only TriLists are handled currently.

	/// Calculates the sphere bounds of the internal vertex data and places it in the provided
	/// Sphere structure.
	static bool WTCall CalculateSphereBounds( CMesh const& M, Math::CSphere& Bounds );

	/// Scales the internal mesh data to fit within a bounding sphere of a specified unit size, optionally re-centering
	/// the mesh data as needed
	static bool WTCall ScaleTo( CMesh& M, float32 Units, bool Center );

private:
	CMeshHelper(); //No construction available
};

inline HMesh WTCall CMesh::CreateCylinder( LPDIRECT3DDEVICE9 pDevice,
		float32 Height, float32 BottomRadius, float32 TopRadius,
		int32 NumVerticalDivisions, int32 NumAnnularDivisions )
{
	HMesh hMesh = new CMesh( pDevice );
	CMeshHelper::SetCylinder( *hMesh, Height, BottomRadius, TopRadius, NumVerticalDivisions, NumAnnularDivisions );
	return hMesh;
}

inline HMesh WTCall CMesh::CreateSphere( LPDIRECT3DDEVICE9 pDevice, float32 Radius, 
	int32 NumLattitudinalDivisions, int32 NumLongitudinalDivisions )
{
	HMesh hMesh = new CMesh( pDevice );
	CMeshHelper::SetSphere( *hMesh, Radius, NumLattitudinalDivisions, NumLongitudinalDivisions );
	return hMesh;
}

inline HMesh WTCall CMesh::CreateSmoothSphere( LPDIRECT3DDEVICE9 pDevice, float32 Radius, int32 NumLevels )
{
	HMesh hMesh = new CMesh( pDevice );
	CMeshHelper::SetSmoothSphere( *hMesh, Radius, NumLevels );
	return hMesh;
}

inline HMesh WTCall CMesh::CreateBox( LPDIRECT3DDEVICE9 pDevice, float32 Width, float32 Height, float32 Depth,
		int32 NumXDivisions, int32 NumYDivisions, int32 NumZDivisions )
{
	HMesh hMesh = new CMesh( pDevice );
	CMeshHelper::SetBox( *hMesh, Width, Height, Depth, NumXDivisions, NumYDivisions, NumZDivisions );
	return hMesh;
}

inline HMesh WTCall CMesh::CreateRect( LPDIRECT3DDEVICE9 pDevice, float32 Width, float32 Height, 
		int32 NumXDivisions, int32 NumYDivisions )
{
	HMesh hMesh = new CMesh( pDevice );
	CMeshHelper::SetRect( *hMesh, Width, Height, NumXDivisions, NumYDivisions );
	return hMesh;
}

} //namespace Graphics
} //namespace WT


namespace WT
{
	DECLARE_VALUETYPE_TYPETRAIT( WT::Graphics::CMesh::GeometryData );
}

template <>
inline void WT::System::Serialization::CSerializationInfo::AddValue( wstring const& name,
	WT::Graphics::CMesh::GeometryData const& value )
{
	AddContainerBegin( name );
	AddValue( L"Position", value.m_Position );
	AddValue( L"Normal", value.m_Normal );
	AddContainerEnd();
}

template <>
inline void WT::System::Serialization::CSerializationInfo::GetValue( wstring const& name,
	WT::Graphics::CMesh::GeometryData& value )
{
	GetContainerBegin( name );
	GetValue( L"Position", value.m_Position );
	GetValue( L"Normal", value.m_Normal );
	GetContainerEnd();
}
