/*
	MeshUA.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace Graphics
{

/// A mesh-related object consisting of an array of unsigned integers
typedef System::SmartPointer<class CMeshUA> HMeshUA;
class CMeshUA : public System::CObject, public System::Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS( CMeshUA, System::CObject );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS();

	// -------------------------------------------------------------
    // ISerializable interface
    // -------------------------------------------------------------

	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

	// -------------------------------------------------------------
    // CMeshUA interface
    // -------------------------------------------------------------

    /// Get the number of unsigned integers.
    int32 GetNumValues() const;

    /// Get the specified unsigned integer.
    bool GetValue(int32 Index, uint32* pValue) const;

    /// Set the number of unsigned integers.
    bool SetNumValues(int32 NumValues);

    /// Set the specified unsigned integer.
    bool SetValue(int32 Index, uint32 Value);

	CMeshUA();
	virtual ~CMeshUA();
	
private:
	std::vector<uint32> m_Values;
};

} //namespace Graphics
} //namespace WT
