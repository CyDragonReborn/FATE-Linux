/***************************************************************************************
 *
 * DspLowPassUnit.h
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

#include "WT/Audio/DspCustomUnit.h"



namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspLowPassUnit> HDspLowPassUnit;
class CDspLowPassUnit : public CDspUnit
{
public:
    
	WT_CLASS_NO_CREATOR( CDspLowPassUnit, CDspUnit );

	CDspLowPassUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspLowPassUnit();

	virtual void					SetActive( bool Active );
	virtual void					SetPriority( int32 Priority );

	virtual bool					GetActive() const;
	virtual int32					GetPriority() const;

	virtual void					SetGain( float32 Gain );
	virtual void					SetCutoffFrequency( float32 Frequency );

	virtual float32					GetGain( ) const;
	virtual float32					GetCutoffFrequency( ) const;

	virtual CAudioFilterLowPass*	GetLowPassFilterLeft() { return &m_LowPassFilterLeft; }
	virtual CAudioFilterLowPass*	GetLowPassFilterRight() { return &m_LowPassFilterRight; }

private:

	CDspCustomUnit*	    m_pCustomDspUnit;
	CAudioFilterLowPass	m_LowPassFilterLeft;
	CAudioFilterLowPass	m_LowPassFilterRight;
};

}	// namespace Audio
}	// namespace WT

