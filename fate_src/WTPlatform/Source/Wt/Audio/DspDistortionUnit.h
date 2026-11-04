/***************************************************************************************
 *
 * DspDistortionUnit.h
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
#include "WT/Audio/DSPFilterFx.h"

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspDistortionUnit> HDspDistortionUnit;
class CDspDistortionUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspDistortionUnit, CDspUnit );

	CDspDistortionUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspDistortionUnit( );

	virtual void						SetActive( bool Active );
	virtual void						SetPriority( int32 Priority );

	virtual bool						GetActive() const;
	virtual int32						GetPriority() const;

	virtual void					SetGain( float32 Gain );
	virtual void					SetDistortion( float32 Distortion );

	virtual float32						GetGain( ) const;
	virtual float32						GetDistortion( ) const;

	virtual CAudioFilterDistortion*	GetDistortionFilterLeft() { return &m_DistortionFilterLeft; }
	virtual CAudioFilterDistortion*	GetDistortionFilterRight() { return &m_DistortionFilterRight; }

private:

	CDspCustomUnit*			m_pCustomDspUnit;
	CAudioFilterDistortion	m_DistortionFilterLeft;
	CAudioFilterDistortion	m_DistortionFilterRight;
};

}	// namespace Audio
}	// namespace WT

