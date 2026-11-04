/*
	MeshFVA.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#pragma once 

#include "WT/System/Object.h"
#include "WT/Mesh/Mesh.h"
#include "WT/Mesh/MeshUA.h"

namespace WT
{

namespace Graphics
{
	class CMeshFVA;
}

template<> struct CCanCastPointer<Graphics::CMeshFVA, System::CReferenceCounted const> {typedef TrueType Result;};

namespace Graphics
{

/// A mesh object consisting of an array of floating-point vectors
typedef System::SmartPointer<class CMeshFVA> HMeshFVA;
class CMeshFVA : public System::CObject, public System::Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS( CMeshFVA, System::CObject );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS();

	// -------------------------------------------------------------
    // ISerializable interface
    // -------------------------------------------------------------

	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void FinalizeDeserialization( System::Serialization::CSerializationInfo* sInfo, System::Serialization::CSerializationContext* sContext );

    // -------------------------------------------------------------
    // CMeshFVA interface
    // -------------------------------------------------------------

	/// Compute an estimate of the serialized size in bytes.
	int32 EstimateSerializedSize( CMesh::SerializationParameters* pParameters ) const;

	/// Get the number of vectors.
    int32 GetNumVectors() const;

    /// Get the number of dimensions.
    int32 GetNumDimensions() const;

    /// Get the specified component.
    float32 GetValue(int32 VectorIndex, int32 DimensionIndex) const
	{
		assert( VectorIndex >= 0 );
		assert( VectorIndex < m_NumVectors );
		assert( DimensionIndex >= 0 );
		assert( DimensionIndex < m_NumDimensions );
		return m_Data[VectorIndex * m_NumDimensions + DimensionIndex];
	}

    /// Set the number of vectors.
    bool SetNumVectors(int32 NumVectors);

    /// Remove one of the vectors. All succeeding vectors will have
    /// their index decreased by 1.
    bool RemoveVector(int32 VectorIndex);

	//Copies the vector with the specified source index to the vector with the specified destination
	//index.
	bool CopyVector( int32 SourceIndex, int32 DestIndex );

    /// Set the number of dimensions. A value in the range of [0,65535] is acceptable.
    bool SetNumDimensions(int32 NumDimensions);

    /// Remove one of the dimensions. All components corresponding to greater dimension values
    /// will be assigned to the previous dimension.
    bool RemoveDimension(int32 DimensionIndex);

    /// Set the specified component.
    void SetValue(int32 VectorIndex, int32 DimensionIndex, float32 Value)
	{
		assert( VectorIndex >= 0 );
		assert( VectorIndex < m_NumVectors );
		assert( DimensionIndex >= 0 );
		assert( DimensionIndex < m_NumDimensions );
	    m_Data[VectorIndex * m_NumDimensions + DimensionIndex] = Value;
	}

	CMeshFVA();
	virtual ~CMeshFVA();

	bool GetUseVQ() const {return m_bUseVQ;}
	void SetUseVQ( bool bUseVQ ) {m_bUseVQ = bUseVQ;}
	uint8 GetPrecision() const {return m_Precision;}
	void SetPrecision( uint8 Precision ) {m_Precision = Precision;}

private:
	//Various kinds of serialization
	void WriteWithVectorQuantization( System::Serialization::CSerializationInfo* SerialInfo ) const;
	void WriteFixedPoint( System::Serialization::CSerializationInfo* SerialInfo, CMesh::SerializationParameters* pParameters ) const;
	void WriteRaw( System::Serialization::CSerializationInfo* SerialInfo ) const;

	void DoIndexing( float32* pNewData, int32& NumUniqueVectors, std::vector<int32>& Remap ) const;
	void WriteFixedPointIndexed( System::Serialization::CSerializationInfo* SerialInfo,
		float32* pMin, float32* pMax, float32* pNewData, int32 NumUniqueVectors, std::vector<int32>& Remap ) const;

    int32 m_NumVectors;
    int32 m_NumDimensions;
	std::vector<float32> m_Data;

	//Temporary storage - data acquired in first stage of deserialization
	//and needed at second stage of deserialization.
	HMeshFVA				m_hCodebook;
	HMeshUA					m_hMap;
	uint8					m_StorageType;
	uint16					m_NumVQDim;
	std::vector<float32>	m_Mins;
	std::vector<float32>	m_Maxs;

	//Parameters of serialization passed during serialization process.
	//(These cannot be passed elgantly through serialization context.)
	uint8 m_Precision;
	bool m_bUseVQ		: 1; //Allow vector quantization compression during serialization
	bool m_bAllowIndex	: 1; //Allow indexing compression during serialization
};

} //namespace Graphics
} //namespace WT
