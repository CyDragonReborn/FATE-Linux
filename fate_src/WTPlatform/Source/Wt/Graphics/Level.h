/***************************************************************************************
 *
 * Level.h
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
#include "WT/Graphics/IUpdateable.h"
#include "WT/Graphics/Model.h"
#include "WT/Graphics/Scene.h"

namespace WT
{
namespace Graphics
{

class CScene;

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/// A CLevel is a collection of polymorphic Model's that provides
/// frustum culling optimizations.
typedef System::SmartPointer<class CLevel> HLevel;
class CLevel : public System::CObject, public System::Serialization::ISerializable, 
	public IDeviceObject, public CRenderableImpl, public IUpdateable
{
public:
	struct Statistics
	{
		int32 m_CullPassed;
		int32 m_CullFailed;

		Statistics() {Clear();}

		void Clear() {m_CullPassed = m_CullFailed = 0;}
	};

	WT_BEGIN_CLASS( CLevel, System::CObject );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( IDeviceObject )
		WT_INHERITS_INTERFACE( IRenderable );
		WT_INHERITS_INTERFACE( IUpdateable );
	WT_END_CLASS();

	CLevel();

	//Create a CLevel. The scene object, if specified, is used for services such as
	//managing reflection textures.
	explicit CLevel( CScene* pScene );

	virtual ~CLevel();

	//
	// IDeviceObject methods
	//
	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	CScene* GetScene() {return m_pScene;}
	CScene const* GetScene() const {return m_pScene;}

	void SetScene( CScene* pScene ) {m_pScene = pScene;}

	/// Test whether model frustum culling is enabled.
	bool GetFrustumCull() const;

	/// Set whether model frustum culling optimization is enabled. The default is true.
	/// Culling works best when models will sometimes be outside of the view frustum
	/// and your application is limited by the GPU.
	void SetFrustumCull( bool FrustumCull );
	
	/// Test whether the optimization of sorting models front to back is enabled. This
	/// optimization is used when rendering opaque geometry.
	bool GetSortOpaque() const;

	/// Set whether the optimization of sorting models front to back is enabled. This 
	/// optimization is used when rendering opaque geometry. The default is true. 
	/// This optimization works best when you have a number of models with opaque 
	/// geometry, when fill rate is expensive (due to expensive pixel shaders or high 
	/// resolution) and your application is limited by the GPU.
	void SetSortOpaque( bool SortOpaque );

	/// Test whether models will be sorted back to front when rendering translucent
	/// geometry.
	bool GetSortTranslucent() const;

	/// Set whether models will be sorted back to front when rendering translucent 
	/// geometry. The default is true. This setting can reduce visual artifacts when 
	/// drawing multiple models containing translucent geometry.
	void SetSortTranslucent( bool SortTranslucent );
	
	/// Render the level.
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	// Perform needed simulation-time updates.
	virtual void Update( float32 ElapsedTimeSeconds );

	Statistics& GetStatistics() {return m_Statistics;};

	int32 GetModelCount() const {return static_cast<int32>( m_hModels.size() );}

	void ClearModels();

	CModel& GetModel( int32 Index );

	void AddModel( CModel& Model );

	//Return the index of the position in the model list that the specified model
	//is at. If no such position exists, return -1.
	int32 FindModel( CModel const* pModel );

	void RemoveModel( int32 Index );

	//
	// ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext );
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private:
	CScene* m_pScene;

	typedef std::vector<HModel> Models;
	Models m_hModels;

	Statistics m_Statistics;

    bool m_ResourcesValid	: 1;
	bool m_FrustumCull		: 1; //Use frustum culling?
	bool m_SortOpaque		: 1; //Sort opaque models front to back?
	bool m_SortTranslucent	: 1; //Sort translucent models back to front?
};

}
}
