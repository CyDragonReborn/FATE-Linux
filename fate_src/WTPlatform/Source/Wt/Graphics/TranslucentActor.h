/***************************************************************************************
 *
 * TranslucentActor.h
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
#include "WT/Graphics/Actor.h"

namespace WT
{
namespace Graphics
{

//CTranslucentMaterial renders a shared opacity value - default color information (white) is used.
typedef System::SmartPointer<class CTranslucentMaterial> HTranslucentMaterial;
class CTranslucentMaterial : public CMaterial
{
public:
	WT_CLASS_NO_CREATOR( CTranslucentMaterial, CMaterial );

	CTranslucentMaterial( System::SmartPointer<float32> const& hOpacity );

	//Get the opacity. A value in the range [0.0,1.0] is returned.
	float32 GetOpacity() const;

	//
	// CMaterial methods
	//
	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;

private:
	System::SmartPointer<float32> m_hOpacity;
};

//CTranslucentActor is an actor plus a shared opacity value. When
//SetFromActor() is called, the contents of the specified actor
//are shared.
typedef System::SmartPointer<class CTranslucentActor> HTranslucentActor;
class CTranslucentActor : public CActor
{
public:
	WT_CLASS( CTranslucentActor, CActor );

	CTranslucentActor();

	//Get the opacity of the translucent actor. A value in the range [0.0,1.0] is returned.
	float32 GetOpacity() const;

	//Set the opacity of the translucent actor to a value in the range [0.0,1.0].
	void SetOpacity( float32 Opacity );

	//Set this translucent actor to share the contents of the specified actor. 
	//A new mesh, materials array, and materials will be created to enable the opacity 
	//to be used at render time. Note that the opacity will affect only certain mesh 
	//parts - currently those that originally used CStandardMaterial materials, or those 
	//that have no materials.
	void SetFromActor( CActor const& Actor );

	//Get and set the shared pointer to the opacity. This can be used, for example, to 
	//share a single opacity value among multiple translucent actors.
	System::SmartPointer<float32> const& GetOpacityPointer();
	void SetOpacityPointer( System::SmartPointer<float32> const& hOpacity );

private:
	System::SmartPointer<float32> m_hOpacity;
};

}
}
