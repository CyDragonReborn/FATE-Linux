/***************************************************************************************
 *
 * CompositeMaterial.h
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          05/3/2006
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

#include "WT/Graphics/Material.h"

namespace WT
{
namespace Graphics
{

namespace EComposite
{
	enum Type
	{
		Add,
		Subtract,
		Mix,
		Count /// Number of composite types
	};
}

typedef System::SmartPointer<class CCompositeMaterial> HCompositeMaterial;
class CCompositeMaterial : public CMaterial, public System::Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS( CCompositeMaterial, CMaterial )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable )
	WT_END_CLASS()

	//
	//ISerializable methods
	virtual void GetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext);

	CCompositeMaterial();
	~CCompositeMaterial();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;

	virtual std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	bool IsTwoSided() const {return m_bTwoSided;}

	void SetTwoSided( bool bTwoSided ) {m_bTwoSided = bTwoSided;}

	/// Get the diffuse map.
	virtual CMaterialMap* GetDiffuseMap() const
	{
		if (m_SubMaterials.empty())
		{
			return NULL;
		}

		CMaterial* pMaterial = m_SubMaterials[0].m_hMaterial;
		if (pMaterial == NULL)
		{
			return NULL;
		}

		return pMaterial->GetDiffuseMap();
	}

	/// Get the number of submaterials.
	int32 GetSubMaterialCount()
	{
		return static_cast<int32>( m_SubMaterials.size() );
	}

	/// Set the number of submaterials.
	void SetSubMaterialCount( uint32 Count )
	{
		m_SubMaterials.resize( Count );
	}

	/// Get the ith submaterial.
	CMaterial* GetSubMaterial( uint32 Index ) const
	{
		if (Index < 0 || Index >= m_SubMaterials.size())
		{
			throw std::out_of_range( "submaterial index" );
		}

		return m_SubMaterials[Index].m_hMaterial;
	}

	/// Set the ith submaterial.
	void SetSubMaterial( uint32 Index, CMaterial* pMaterial )
	{
		if (Index < 0 || Index >= m_SubMaterials.size())
		{
			throw std::out_of_range( "submaterial index" );
		}

		m_SubMaterials[Index].m_hMaterial = pMaterial;
	}

	/// Get the composite type for the ith submaterial. The value returned is a cmEComposite value.
	int32 GetCompositeType( uint32 Index ) const
	{
		if (Index < 0 || Index >= m_SubMaterials.size())
		{
			throw std::out_of_range( "submaterial index" );
		}

		return m_SubMaterials[Index].m_CompositeType;
	}

	/// Set the composite type for the ith submaterial. Supply a cmEComposite value for CompositeType.
	void SetCompositeType( uint32 Index, EComposite::Type CompositeType )
	{
		if (Index < 0 || Index >= m_SubMaterials.size())
		{
			throw std::out_of_range( "submaterial index" );
		}

		if (CompositeType < 0 || CompositeType >= EComposite::Count)
		{
			throw std::invalid_argument( "composite type" );
		}

		m_SubMaterials[Index].m_CompositeType = CompositeType;
	}

	/// Get the composite amount for the ith submaterial as a percentage
	float32 GetCompositeAmount( uint32 Index ) const
	{
		if (Index < 0 || Index >= m_SubMaterials.size())
		{
			throw std::out_of_range( "submaterial index" );
		}

		return m_SubMaterials[Index].m_CompositeAmount;
	}

	/// Set the composite amount for the ith submaterial as a percentage
	void SetCompositeAmount( uint32 Index, float32 CompositeAmount )
	{
		if (Index < 0 || Index >= m_SubMaterials.size())
		{
			throw std::out_of_range( "submaterial index" );
		}

		m_SubMaterials[Index].m_CompositeAmount = CompositeAmount;
	}

private:

	struct SubMaterial
	{
		HMaterial			m_hMaterial;
		EComposite::Type	m_CompositeType;
		float32				m_CompositeAmount; //Using [0.0,1.0] scale

		SubMaterial()
			: m_CompositeType( EComposite::Add )
			, m_CompositeAmount( 0.0f )
		{
		}
	};

	std::string m_Name;
	bool m_ResourcesValid : 1;
	bool m_bTwoSided : 1;
	vector<SubMaterial> m_SubMaterials;
};

}
}
