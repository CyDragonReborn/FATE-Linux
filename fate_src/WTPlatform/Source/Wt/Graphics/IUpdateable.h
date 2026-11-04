/*
	IUpdateable.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/28/2005
*/

#pragma once

#include "WT/System/IInterface.h"

namespace WT
{
	namespace Graphics
	{
		// Interface to an updateable object.
		class IUpdateable : public System::IInterface
		{
			public:
				WT_INTERFACE( IUpdateable );

				//Perform needed simulation-time updates.
				virtual void Update( float32 ElapsedTimeSeconds ) = 0;
		};
	}
}
