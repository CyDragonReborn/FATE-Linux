/***************************************************************************************
 *
 * DspWaveformUnit.h
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

#include "WT/System/Object.h"
#include "WT/Audio/DspCustomUnit.h"

namespace WT
{
namespace Audio
{
	
enum EAudioChannel
{
    KLeftChannel = 0,
    KRightChannel,
    KCenterChannel,
    KNumChannels
};

typedef System::SmartPointer<class CDspWaveformUnit> HDspWaveformUnit;
class CDspWaveformUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspWaveformUnit, CDspUnit );

    CDspWaveformUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspWaveformUnit();

	virtual void				SetActive( bool Active );
	virtual void				SetPriority( int32 Priority );

	virtual bool				GetActive() const;
	virtual int32				GetPriority() const;

	virtual const void*			GetWaveform( EAudioChannel Channel ) const;

	virtual void*				GetWaveform( EAudioChannel Channel );

private:

	CDspCustomUnit*			m_pCustomDspUnit;
	void*					m_WaveformData[ KNumChannels ];
};

}	// namespace Audio
}	// namespace WT

