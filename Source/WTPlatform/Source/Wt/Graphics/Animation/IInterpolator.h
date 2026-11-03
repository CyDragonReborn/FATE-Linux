/*
	IInterpolator.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/IInterface.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

class IInterpolator : public WT::System::IInterface
{
public:
	WT_BEGIN_INTERFACE( IInterpolator )
		WT_INHERITS_INTERFACE( WT::System::IInterface )
	WT_END_INTERFACE()

	//
	// IInterpolator interface
	//

	virtual void Update(float32 Time) = 0;

};


} // namespace Animation
} // namespace Graphics
} // namespace WT
