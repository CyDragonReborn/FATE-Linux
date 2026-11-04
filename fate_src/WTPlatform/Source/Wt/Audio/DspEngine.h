/***************************************************************************************
 *
 * DspEngine.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/17/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/Audio/DspUnit.h"
#include "WT/Audio/DspFFTUnit.h"
#include "WT/Audio/AudioStream.h"
#include "WT/Audio/AudioCD.h"
#include "WT/Audio/DspCustomUnit.h"
#include "WT/Audio/DspSystem.h"

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspEngine> HDspEngine;
class CDspEngine : public System::CObject
{
public:

	WT_CLASS( CDspEngine, System::CObject );

    CDspEngine();
	virtual ~CDspEngine( );

	virtual int32				GetMixRateHz( ) const;
	virtual int32				GetDspBufferLength( ) const;
	virtual EMixerSampleType	GetMixerSampleType( ) const;
	virtual void				ClearMixBuffer( );
	virtual int32				GetMixBufferSize( ) const;
	virtual void				MixBuffers(	void* DestBuffer,
											void* SrcBuffer,
											int32 Length,
											int32 Frequency,
											float32 Volume,
											float32 Pan,
											uint32 Mode );

};

}	// namespace Audio
}	// namespace WT

