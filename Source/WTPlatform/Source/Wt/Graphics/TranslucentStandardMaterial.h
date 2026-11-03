/***************************************************************************************
 *
 * TranslucentStandardMaterial.h
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

#include "WT/Graphics/StandardMaterial.h"
#include "WT/Graphics/TranslucentActor.h"

namespace WT
{
namespace Graphics
{

//CTranslucentStandardMaterial is a proxy for a CStandardMaterial that adds the ability to apply a 
//shared opacity value.
typedef System::SmartPointer<class CTranslucentStandardMaterial> HTranslucentStandardMaterial;
class CTranslucentStandardMaterial : public CTranslucentMaterial
{
public:
	WT_CLASS_NO_CREATOR( CTranslucentStandardMaterial, CTranslucentMaterial );

	CTranslucentStandardMaterial( CStandardMaterial const* pStandardMaterial, System::SmartPointer<float32> const& hOpacity );

	CStandardMaterial const* GetStandardMaterial() const;

	//
	// CMaterial methods
	//
	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;

private:
	System::SmartPointer<CStandardMaterial const> m_hStandardMaterial;
};

}
}
