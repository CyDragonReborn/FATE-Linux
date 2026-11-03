/*
	MeshVtxPropCol.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/SerializationInfo.h"

namespace WT
{
namespace Graphics
{

/// A collection of vertex properties
class CMeshVtxPropCol
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
    // CMeshVtxPropCol interface
    // -------------------------------------------------------------

    /// Get the number of properties in this collection.
    int32 GetPropertyCount() const {return static_cast<int32>( m_Props.size() );}
 
    /// Get the specified property from this collection.
    CMeshVtxProp* GetProperty(int32 Index);

    /// Get the specified property from this collection.
    CMeshVtxProp* GetProperty(int32 Index) const;

    /// Remove all properties from this collection.
    bool ClearProperties();

    /// Set the number of properties in this collection. A value in 
    /// the range [0,255] is acceptable. Newly allocated properties will be 
    /// NULL pointers and must be filled in manually.
    bool SetPropertyCount(int32 PropertyCount);

    /// Remove the specified property from this collection.
    bool RemoveProperty(int32 Index);
    
    /// Set the specified property.
    bool SetProperty(int32 Index, CMeshVtxProp* pProperty);

    /// Get the property value for the specified vertex. PropIndex specifies the index
    /// of the property with respect to the property collection. Warning: this may return a 
    /// successful result even if you exceed the number of vertices in the mesh.
    bool GetValue(int32 PropIndex, int32 VertexIndex, float32* pValue) const;

    /// Set the property value for the specified vertex. PropIndex specifies the index
    /// of the property with respect to the property collection. Warning: this may return
    /// a successful result even if you exceed the number of vertices in the mesh.
    bool SetValue(int32 PropIndex, int32 VertexIndex, float32 Value);

	//Maximum number of properties allowed
	static int32 const KMaxPropertyCount = 255;

    ~CMeshVtxPropCol();

private:
	std::vector<CMeshVtxProp*> m_Props;
};

} //namespace Graphics
} //namespace WT
