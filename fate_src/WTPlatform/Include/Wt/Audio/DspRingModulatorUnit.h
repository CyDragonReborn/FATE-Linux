/***************************************************************************************
 *
 * DspRingModulatorUnit.h
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

typedef System::SmartPointer<class CDspRingModulatorUnit> HDspRingModulatorUnit;
class CDspRingModulatorUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspRingModulatorUnit, CDspUnit );

	CDspRingModulatorUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspRingModulatorUnit();
	
	virtual void						SetActive( bool Active );
	virtual void						SetPriority( int32 Priority );

	virtual bool						GetActive() const;
	virtual int32						GetPriority() const;

	virtual void					SetGain( float32 Gain );
	virtual void					SetFrequency( float32 Frequency );

	virtual float32						GetGain( ) const;
	virtual float32						GetFrequency( ) const;

	virtual CAudioFilterRingModulator* GetRingModulatorFilterLeft() { return &m_RingModulatorFilterLeft; }
	virtual CAudioFilterRingModulator*	GetRingModulatorFilterRight() { return &m_RingModulatorFilterRight; }

private:

	CDspCustomUnit*				m_pCustomDspUnit;
	CAudioFilterRingModulator	m_RingModulatorFilterLeft;
	CAudioFilterRingModulator	m_RingModulatorFilterRight;
  
};

}	// namespace Audio
}	// namespace WT

