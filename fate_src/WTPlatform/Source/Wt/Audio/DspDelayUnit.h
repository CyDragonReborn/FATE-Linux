/***************************************************************************************
 *
 * DspDelayUnit.h
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

typedef System::SmartPointer<class CDspDelayUnit> HDspDelayUnit;
class CDspDelayUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspDelayUnit, CDspUnit );

    CDspDelayUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspDelayUnit();

	virtual void				SetActive( bool Active );
	virtual void				SetPriority( int32 Priority );

	virtual bool				GetActive() const;
	virtual int32				GetPriority() const;

	virtual void			SetBufferTime( float32 Seconds );
	virtual void			SetDelayTime( float32 Seconds );
	virtual void			SetGain( float32 Gain );

	virtual float32				GetBufferTime( ) const;
	virtual float32				GetDelayTime( ) const;
	virtual float32				GetGain( ) const;

	virtual CAudioFilterDelay*	GetDelayFilterLeft() { return &m_DelayFilterLeft; }
	virtual CAudioFilterDelay*	GetDelayFilterRight() { return &m_DelayFilterRight; }

private:

	CDspCustomUnit*			m_pCustomDspUnit;
	CAudioFilterDelay		m_DelayFilterLeft;
	CAudioFilterDelay		m_DelayFilterRight;

};

}	// namespace Audio
}	// namespace WT

