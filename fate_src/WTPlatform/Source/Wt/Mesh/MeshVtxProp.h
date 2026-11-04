/*
	MeshVtxProp.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once

#include "WT/Mesh/MeshFVA.h"

namespace WT
{
namespace Graphics
{

/// The type of a vertex property. Properties take on per-vertex
/// floating-point values defaulting to 0.0. Exceptions include colors which
/// default to 255.0.
enum EVtxPropType
{
    KVtxPropTypePositionX = 0, /// X-coordinate of position vector
    KVtxPropTypePositionY = 1, /// Y-coordinate of position vector
    KVtxPropTypePositionZ = 2, /// Z-coordinate of position vector
    KVtxPropTypeNormalX = 4, /// X-coordinate of normal vector
    KVtxPropTypeNormalY = 5, /// Y-coordinate of normal vector
    KVtxPropTypeNormalZ = 6, /// Z-coordinate of normal vector
    KVtxPropTypeColorR = 8, /// R-component of color (typically in [0.0,255.0] range)
    KVtxPropTypeColorG = 9, /// G-component of color (typically in [0.0,255.0] range)
    KVtxPropTypeColorB = 10, /// B-component of color (typically in [0.0,255.0] range)
    KVtxPropTypeColorA = 11, /// Alpha (opacity) component of color (typically in [0.0,255.0] range)
    KVtxPropTypeTexture1U = 32, /// U texture coordinate for first texture
    KVtxPropTypeTexture1V = 33, /// V texture coordinate for first texture
    KVtxPropTypeTexture2U = 34, /// U texture coordinate for second texture
    KVtxPropTypeTexture2V = 35, /// V texture coordinate for second texture
    /// (Sixteen UV sets are supported, using property type values ranging from
    /// 32 through 63.)
    KVtxPropTypeTextureLastU = 62, /// U texture coordinate for sixteenth texture
    KVtxPropTypeTextureLastV = 63, /// V texture coordinate for sixteenth texture

	//For bone indices, only nonnegative integral values are fully supported.
	//There is support for one more bone index than there are bone weights, because
	//it is always assumed that there is a last weight that brings the total weight 
	//to 1.0.
	KVtxPropTypeBoneIndex1 = 64,   /// Index of first bone. Defaults to 0.
	KVtxPropTypeBoneWeight1 = 65,  /// Weight of first bone. Defaults to 0.0.
	KVtxPropTypeBoneIndex2 = 66,   /// Index of second bone. Defaults to 0.
	KVtxPropTypeBoneWeight2 = 67,  /// Weight of second bone. Defaults to 0.0.
	KVtxPropTypeBoneIndex3 = 68,   /// Index of third bone. Defaults to 0.
	KVtxPropTypeBoneWeight3 = 69,  /// Weight of third bone. Defaults to 0.0.
	KVtxPropTypeBoneIndex4 = 70,   /// Index of fourth bone. Defaults to 0.

    KVtxPropTypeUserStart = 0x0080u, /// User-defined properties start at this point

    KVtxPropTypeMax = 0xffffu /// The maximum permissible user-defined property type value
};

/// A property that takes on values for zero or more vertices
class CMeshVtxProp
{
public:
	// -------------------------------------------------------------
    // Serialization-related functions
    // -------------------------------------------------------------

    /// Deserialize the object from the stream.
	void Read( System::Serialization::CSerializationInfo* sInfo );

    /// Serialize the object to the stream.
    void Write( System::Serialization::CSerializationInfo* sInfo );

    // -------------------------------------------------------------
    // CMeshVtxProp interface
    // -------------------------------------------------------------

	/// Get the type of this vertex property
    int32 GetPropType() const {return m_PropType;}

    /// Get the FVA object that provides the data for this property.
    /// Note that the number of vertices for which the data is 
    /// provided actually is specified by the mesh using this property.
    CMeshFVA* GetData() {return m_hFVA;}

    /// Get the FVA object that provides the data for this property.
    /// Note that the number of vertices for which the data is 
    /// provided actually is specified by the mesh using this property.
    CMeshFVA const* GetData() const {return m_hFVA;}

    /// Get the vector index that is referred to in the FVA (floating-point
    /// vector array). Additional vertices will use data from subsequent
    /// vector indices.
    int32 GetVectorIndex() const {return m_VectorIndex;}

    /// Get the dimension index that is referred to in the FVA (floating-point
    /// vector array). Additional vertices will use data from subsequent
    /// dimension indices.
    int32 GetDimensionIndex() const {return m_DimensionIndex;}

    /// Set the type of this vertex property. A value should be 
    /// chosen from the cmEVtxPropType enumeration.
    bool SetPropType( int32 PropType );
        
    /// Set the FVA object that provides the data for this property.
    /// Note that the number of vertices for which the data is 
    /// provided actually is specified by the mesh using this property.
    void SetData( CMeshFVA* pData ) {m_hFVA = pData;}

    /// Set the vector index that is referred to in the FVA (floating-point
    /// vector array). Additional vertices will use data from subsequent
    /// vector indices.
    void SetVectorIndex( int32 VectorIndex ) {m_VectorIndex = VectorIndex;}

    /// Set the dimension index that is referred to in the FVA (floating-point
    /// vector array). Additional vertices will use data from subsequent
    /// dimension indices. Values in the range [0,65535] are allowed.
    bool SetDimensionIndex( int32 DimensionIndex );

    /// Get the property value for the specified vertex. Warning: this may return a 
    /// successful result even if you exceed the number of vertices in the mesh.
    bool GetValue( int32 VertexIndex, float32* pValue ) const;

    /// Set the property value for the specified vertex. Warning: this may return
    /// a successful result even if you exceed the number of vertices in the mesh.
    bool SetValue( int32 VertexIndex, float32 Value );

	CMeshVtxProp();

private:
    int32 m_PropType;
    HMeshFVA m_hFVA;
    int32 m_VectorIndex;
    int32 m_DimensionIndex;
};

} //namespace Graphics
} //namespace WT
