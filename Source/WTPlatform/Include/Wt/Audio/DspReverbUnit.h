/***************************************************************************************
 *
 * DspReverbUnit.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/20/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/DspCustomUnit.h"



namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspReverbUnit> HDspReverbUnit;
class CDspReverbUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspReverbUnit, CDspUnit );

	CDspReverbUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspReverbUnit();

	virtual void						SetActive( bool Active );
	virtual void						SetPriority( int32 Priority );

	virtual bool						GetActive() const;
	virtual int32						GetPriority() const;

	virtual void					SetBufferTime( float32 Seconds );
	virtual void					SetDelayTime( float32 Seconds );
	virtual void					SetGain( float32 Gain );
	virtual void					SetDelayFactor( float32 DelayFactor );

	virtual float32						GetBufferTime( ) const;
	virtual float32						GetDelayTime( ) const;
	virtual float32						GetGain( ) const;
	virtual float32						GetDelayFactor( ) const;

	virtual CAudioFilterReverberation*	GetReverbFilterLeft() { return &m_ReverbFilterLeft; }
	virtual CAudioFilterReverberation*	GetReverbFilterRight() { return &m_ReverbFilterRight; }

private:

	CDspCustomUnit*					m_pCustomDspUnit;
	CAudioFilterReverberation		m_ReverbFilterLeft;
	CAudioFilterReverberation		m_ReverbFilterRight;
};

}	// namespace Audio
}	// namespace WT

