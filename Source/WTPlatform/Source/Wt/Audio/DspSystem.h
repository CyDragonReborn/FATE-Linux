/***************************************************************************************
 *
 * DspSystem.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/17/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/DspUnit.h"
#include "WT/Audio/DspFFTUnit.h"
#include "WT/Audio/AudioStream.h"
#include "WT/Audio/AudioCD.h"

namespace WT
{
namespace Audio
{

class CDspEngine;

enum EMixerSampleType
{
	KInt16StereoSample,
	KFloat32StereoSample
};

typedef System::SmartPointer<class CDspSystem> HDspSystem;
class CDspSystem : public System::CObject
{
public:

	WT_CLASS_NO_CREATOR( CDspSystem, System::CObject );

    CDspSystem( CDspEngine* pDspEngine );
	virtual ~CDspSystem( );

	/*virtual HDspUnit		CreateDspUnit(	const System::Type* pDspUnitId,
											int32 Priority,
											Object* pStreamOrCD );*/

	virtual int32				GetMixRateHz( ) const;
	virtual int32				GetDspBufferLength( ) const;
	virtual EMixerSampleType	GetMixerSampleType( ) const;
	virtual void				ClearMixBuffer( );
	virtual int32				GetMixBufferSize( ) const;
	virtual void			MixBuffers(	void* DestBuffer,
											void* SrcBuffer,
											int32 Length,
											int32 Frequency,
											float32 Volume,
											float32 Pan,
											uint32 Mode );

private:

	CDspEngine*			m_pDspEngine;
};

}	// namespace Audio
}	// namespace WT
