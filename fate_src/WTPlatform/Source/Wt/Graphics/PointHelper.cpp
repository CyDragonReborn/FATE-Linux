/***************************************************************************************
 *
 * PointHelper.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 10/27/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Graphics/PointHelper.h"

using namespace WT::System;
using namespace WT::System::Serialization;
using namespace WT::Math;
using namespace WT::Graphics;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CPointHelper )

uint32 WTCall CPointHelper::GetClassFormatVersion() {return 4;}

CPointHelper::CPointHelper()
	: m_hProperties( new CUserPropertyCollection() )
{
}

/*virtual*/ void CPointHelper::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	GetClassType()->GetProperties()->SerializePropertyValues( this, Info, sContext);

	Info->AddValue( L"properties", *m_hProperties );
}

/*virtual*/ void CPointHelper::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	if (Info->GetObjectFormatVersion() < 4)
	{
		throw std::runtime_error( "unsupported CPointHelper data version" );
	}

	//Initialize property values
	SetTransform( CMatrix4f::gm_Identity );
	SetScaleComponent( Math::CVector3f::gm_UnitScale );
	SetRotationComponent( CQuaternion() );

	GetClassType()->GetProperties()->DeserializePropertyValues( this, Info, sContext);

	Info->GetValue( L"properties", *m_hProperties );
}

/*virtual*/ void CPointHelper::FinalizeDeserialization(CSerializationInfo* Info, CSerializationContext* sContext)
{
	//_BaseClass::FinalizeDeserialization( Info, sContext );
}

} //namespace Graphics

} //namespace WT
