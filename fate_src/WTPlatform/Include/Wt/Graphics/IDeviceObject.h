/*
	IDeviceObject.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#pragma once

#include "d3d9.h"
#include "WT/System/IInterface.h"

namespace WT
{
namespace Graphics
{

// Device object is an abstract base class to represent objects that have D3D device
// dependencies (like VBs, IBs, etc.) this class provides the ability to invalidate
// and restore the device objects, which will be called by the framework (or by the
// user if needed).

class IDeviceObject : public System::IInterface
{
public:
	WT_INTERFACE( IDeviceObject )

	virtual void InvalidateDeviceObjects() = 0;
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice ) = 0;
	virtual bool IsValid() const = 0;
};

}
}
