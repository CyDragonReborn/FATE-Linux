/***************************************************************************************
 *
 * DspFlangeUnit.h
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

#include "WT/Audio/DspCustomUnit.h"

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspFlangeUnit> HDspFlangeUnit;
class CDspFlangeUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspFlangeUnit, CDspUnit );

    CDspFlangeUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspFlangeUnit();

	virtual void					SetActive( bool Active );
	virtual void					SetPriority( int32 Priority );

	virtual bool					GetActive() const;
	virtual int32					GetPriority() const;

    virtual void				SetMinDelayTime(float32 MinDelayTime);
    virtual void				SetMaxDelayTime(float32 MaxDelayTime);
    virtual void				SetFrequency(float32 Frequency);

    virtual float32					GetMinDelayTime() const;
    virtual	float32					GetMaxDelayTime() const;
    virtual float32					GetFrequency() const;

	virtual CAudioFilterFlange*	GetFlangeFilterLeft() { return &m_FlangeFilterLeft; }
	virtual CAudioFilterFlange*	GetFlangeFilterRight() { return &m_FlangeFilterRight; }

private:

	CDspCustomUnit*			m_pCustomDspUnit;
	CAudioFilterFlange		m_FlangeFilterLeft;
	CAudioFilterFlange		m_FlangeFilterRight;
};

}	// namespace Audio
}	// namespace WT

