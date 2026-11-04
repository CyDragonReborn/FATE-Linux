/***************************************************************************************
 *
 * Model.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          8/2/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Object.h"
#include "WT/System/SmartPointer.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Math/Math.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Mesh/Mesh.h"
#include "WT/Graphics/Animation/Skeleton.h"
#include "WT/Graphics/Animation/Pose.h"
#include "WT/Graphics/Animation/Motion.h"
#include "WT/Graphics/Material.h"
#include "WT/Graphics/IMovable.h"

namespace WT
{
namespace Graphics
{

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/// A CModel is a rigid object that can be moved and rendered.
typedef System::SmartPointer<class CModel> HModel;
class CModel : public System::CObject, public CRenderableImpl, public CMovableImpl, public System::Serialization::ISerializable,
	public IDeviceObject	
{
public:
	WT_BEGIN_CLASS( CModel, System::CObject );
		WT_INHERITS_INTERFACE( IMovable );
		WT_INHERITS_INTERFACE( IRenderable );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( IDeviceObject );

		WT_BEGIN_PROPERTIES();
			WT_PROP_RW_BYREF( Name, string );
		WT_END_PROPERTIES();
	WT_END_CLASS();

	CModel();
	CModel( CMesh* pMesh );
	virtual ~CModel();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	/// Render the model.
	//Note: because of rendering categories (ERenderCategory), Render() is typically called three times.
	//This results in redundant render state changes (e.g. world matrices for models and actors), frustum
	//culling, sorting, and possibly other work. TODO optimize this by sharing precomputed information,
	//testing sub-objects for need to render under particular render categories, or depending on the user
	//to manage this issue.
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	//Perform needed simulation-time updates.
	virtual void Update( float32 ElapsedTimeSeconds ) {ElapsedTimeSeconds;}

	/// Get the mesh
	CMesh* GetMesh() {return m_hMesh;}
	CMesh const* GetMesh() const {return m_hMesh;}

	/// Set the mesh
	void SetMesh( CMesh* pMesh );

	//Perform mesh optimizations.
	virtual void Optimize();

	System::SmartPointer< std::vector<HMaterial> > const& GetMaterials() const;

	void SetMaterials( System::SmartPointer<std::vector<HMaterial> > const& hMaterials );

	CMaterial* FindMaterialByName( std::string const& Name ) const;

	//Get a bounding box in the model's coordinate system.
	Math::CAABox const& GetBoundingBox() const {return m_BoundingBox;}

	//Set the bounding box in the model's coordinate system
	//This should be large enough to accomodate the final
	//animated, morphed version of the mesh. Deserializing
	//a Model will result in the calculation of a suggested
	//bounding box.
	void SetBoundingBox( Math::CAABox const& Box ) {m_BoundingBox = Box;}

	//Compute a bounding box for the model. This is designed for static models and
	//doesn't take into account morph target blending or animation.
	void ComputeBoundingBox();

	//Check whether this Model intersects the frustum. This is a cheap
	//method and may return false positives.
	//Note: when processing a hierarchy of objects, you should keep track of
	//flags indicating which of the six frustum clipping planes are satisfied. 
	//This function is intended for a single object only.
	bool Intersects( CFrustum const& frustum ) const;

	//
	// ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private:
	HMesh											m_hMesh;
	System::SmartPointer< std::vector<HMaterial> >	m_hMaterials;
	Math::CAABox									m_BoundingBox;
	std::string										m_Name;
	bool											m_ResourcesValid : 1;
};

}
}
