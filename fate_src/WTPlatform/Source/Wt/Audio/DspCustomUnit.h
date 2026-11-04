/***************************************************************************************
 *
 * DspCustomUnit.h
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

#include "WT/Audio/DspSystem.h"
#include "WT/Audio/DSPFilterFx.h"

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspCustomUnit> HDspCustomUnit;
class CDspCustomUnit : public CDspUnit
{
public:

	WT_CLASS_NO_CREATOR( CDspCustomUnit, CDspUnit );

    CDspCustomUnit( DspCallback Cb, int32 Priority, int32 Param, CObject* pStreamOrCD );
	virtual ~CDspCustomUnit( );

	virtual void					SetActive( bool Active );
	virtual void					SetPriority( int32 Priority );

	virtual bool					GetActive() const;
	virtual int32					GetPriority() const;

	virtual void					GetDspCallback( DspCallback& Cb, int32& Param );

private:

	FSOUND_DSPUNIT*		m_pDspUnit;
	DspCallback			m_Cb;
	int32				m_CbParam;
};

}	// namespace Audio
}	// namespace WT

