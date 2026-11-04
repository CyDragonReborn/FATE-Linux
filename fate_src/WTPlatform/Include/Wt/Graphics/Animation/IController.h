/*
	IController.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	4/3/2005
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


class IController : public IInterface
{
public:
	WT_BEGIN_INTERFACE( IController )
		WT_INHERITS_INTERFACE( IInterface )
	WT_END_INTERFACE()

	//inputs

	//TODO Implement properties
	virtual void SetInput(uint32 Index /*, const cmProperty* Property*/) = 0;
	virtual uint32 GetInputsCount() = 0;

	//outputs

	//TODO Implement properties
	virtual void SetOutput(uint32 Index /*, const cmProperty* Property*/ ) = 0;
	virtual uint32 GetOutputsCount();

	virtual void Update(float32 FrameTime) = 0;
};


} // Animation
} // Graphics
} // WT
