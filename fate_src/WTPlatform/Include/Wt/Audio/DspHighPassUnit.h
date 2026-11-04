/***************************************************************************************
 *
 * DspHighPassUnit.h
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

typedef System::SmartPointer<class CDspHighPassUnit> HDspHighPassUnit;
class CDspHighPassUnit : public CDspUnit
{
public:
    
	WT_CLASS_NO_CREATOR( CDspHighPassUnit, CDspUnit );

	CDspHighPassUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspHighPassUnit();
	
	virtual void					SetActive( bool Active );
	virtual void					SetPriority( int32 Priority );

	virtual bool					GetActive() const;
	virtual int32					GetPriority() const;

	virtual void					SetGain( float32 Gain );
	virtual void					SetCutoffFrequency( float32 Frequency );

	virtual float32					GetGain( ) const;
	virtual float32					GetCutoffFrequency( ) const;

	virtual CAudioFilterHighPass*	GetHighPassFilterLeft() { return &m_HighPassFilterLeft; }
	virtual CAudioFilterHighPass*	GetHighPassFilterRight() { return &m_HighPassFilterRight; }

private:

	CDspCustomUnit*		m_pCustomDspUnit;
	CAudioFilterHighPass	m_HighPassFilterLeft;
	CAudioFilterHighPass	m_HighPassFilterRight;
};

}	// namespace Audio
}	// namespace WT

