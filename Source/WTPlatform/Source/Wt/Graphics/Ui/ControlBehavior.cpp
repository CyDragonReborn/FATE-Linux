//
// ControlBehavior.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/ControlBehavior.h"
#include "WT/Graphics/UI/Control.h"


namespace WT
{
namespace Graphics
{
namespace UI
{


// Determines whether the given control should be filtered from update because its current
// state is not a state this behavior modifier instance controls
bool CControlBehavior::FilterUpdate( CControl* pControl, IDirect3DDevice9* pDevice )
{
	pDevice;

	if( pControl == NULL )
	{
		return true;
	}

	uint32 ControlState = 0;
	bool ContinueLoop = false;

	do
	{
		if( !pControl->GetEnabled( ) )
		{
			ControlState = EControlState::Disabled;
			break;
		}
		if( !pControl->GetVisible( ) )
		{
			ControlState = EControlState::Hidden;
			break;
		}			
		if( pControl->IsPressed( ) )
		{
			ControlState = EControlState::Pressed;
			break;
		}			
		if( pControl->HasMouseOver( ) )
		{
			ControlState = EControlState::MouseOver;
			break;
		}
		if( pControl->GetFocus( ) )
		{
			ControlState = (EControlState::Normal | EControlState::Focus);
			break;
		}

		ControlState = EControlState::Normal;
		break;

	}	while( ContinueLoop );

	return (ControlState & m_ControlStates) == 0 ? true : false;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
