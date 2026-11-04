/***************************************************************************************
 *
 * DspUnit.h
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 12/08/2003: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"

struct FSOUND_DSPUNIT;

namespace WT
{
namespace Audio
{

typedef void * (__cdecl *DspCallback) (void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param);

typedef System::SmartPointer<class CDspUnit> HDspUnit;
class CDspUnit : public System::CObject
{
public:
   
	WT_CLASS( CDspUnit, System::CObject );

	virtual void			SetActive( bool Active ) { assert(0); Active; }
	virtual void			SetPriority( int32 Priority ) { assert(0); Priority; }

	virtual bool			GetActive() const  { assert(0); return false; }
	virtual int32			GetPriority() const  { assert(0); return 0; }
};

}	// namespace Audio
}	// namespace WT

