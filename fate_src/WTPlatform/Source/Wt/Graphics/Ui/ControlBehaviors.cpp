//
// ControlBehaviors.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/ControlBehaviors.h"


namespace WT
{
namespace Graphics
{
namespace UI
{


//
// ControlTransform
//

void CControlTransform::UpdateControlInternal( CControl* pControl, IDirect3DDevice9* )
{
	if( m_AppliedOffsets == 0 )
		return;

	if( m_AppliedOffsets & ESizeOffset )
	{
		m_RestoreSize = pControl->GetSize( );
		pControl->SetSize( m_RestoreSize + m_SizeOffset );
	}
	if( m_AppliedOffsets & EPositionOffset )
	{
		m_RestorePosition = pControl->GetPosition( );
		pControl->SetPosition( m_RestorePosition + m_PositionOffset );
	}
	if( m_AppliedOffsets & ECenterRotationOffset )
	{
		m_RestoreCenterRotation = pControl->GetRotationCenter( );
		pControl->SetRotationCenter( m_RestoreCenterRotation + m_CenterRotationOffset );
	}
	if( m_AppliedOffsets & EZRotationOffset )
	{
		m_RestoreZRotation = pControl->GetRotationZ( );
		pControl->SetRotationZ( m_RestoreZRotation + m_ZRotationOffset );
	}		
}

void CControlTransform::RestoreControlInternal( CControl* pControl, IDirect3DDevice9* )
{
	if( m_AppliedOffsets == 0 )
		return;

	if( m_AppliedOffsets & ESizeOffset )
	{
		pControl->SetSize( m_RestoreSize );
	}
	if( m_AppliedOffsets & EPositionOffset )
	{
		pControl->SetPosition( m_RestorePosition );
	}
	if( m_AppliedOffsets & ECenterRotationOffset )
	{
		pControl->SetRotationCenter( m_RestoreCenterRotation );
	}
	if( m_AppliedOffsets & EZRotationOffset )
	{
		pControl->SetRotationZ( m_RestoreZRotation );
	}	
}

void CControlTransform::CopyPropertiesFrom( const CControlTransform& Other )
{
	CControlBehavior::CopyPropertiesFrom( Other );

	m_SizeOffset = Other.m_SizeOffset;
	m_PositionOffset = Other.m_PositionOffset;
	m_CenterRotationOffset = Other.m_CenterRotationOffset;
	m_ZRotationOffset = Other.m_ZRotationOffset;

	m_RestoreSize = Other.m_RestoreSize;
	m_RestorePosition = Other.m_RestorePosition;
	m_RestoreCenterRotation = Other.m_RestoreCenterRotation;
	m_RestoreZRotation = Other.m_RestoreZRotation;
	m_AppliedOffsets = Other.m_AppliedOffsets;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
