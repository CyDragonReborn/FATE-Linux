/***************************************************************************************
 *
 * DspChorusUnit.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/19/2004: Jason Glover:	Creation
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

typedef System::SmartPointer<class CDspChorusUnit> HDspChorusUnit;
class CDspChorusUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspChorusUnit, CDspUnit );
   
	CDspChorusUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspChorusUnit( );

	virtual void					SetActive( bool Active );
	virtual void					SetPriority( int32 Priority );

	virtual bool					GetActive() const;
	virtual int32					GetPriority() const;

	virtual void				SetBufferTime( float32 Seconds );
	virtual void				SetDelayTime( float32 Seconds );
	virtual void				SetGain( float32 Gain );

	virtual float32					GetBufferTime( ) const;
	virtual float32					GetDelayTime( ) const;
	virtual float32					GetGain( ) const;

	virtual CAudioFilterChorus*	GetChorusFilterLeft() { return &m_ChorusFilterLeft; }
	virtual CAudioFilterChorus*	GetChorusFilterRight() { return &m_ChorusFilterRight; }
	
private:

	CDspCustomUnit*			m_pCustomDspUnit;
	CAudioFilterChorus		m_ChorusFilterLeft;
	CAudioFilterChorus		m_ChorusFilterRight;
};

}	// namespace Audio
}	// namespace WT

