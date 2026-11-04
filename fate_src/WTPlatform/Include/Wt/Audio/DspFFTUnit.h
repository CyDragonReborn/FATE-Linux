/***************************************************************************************
 *
 * DspFFTUnit.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/19/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/Audio/DspUnit.h"

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspFFTUnit> HDspFFTUnit;
class CDspFFTUnit : public CDspUnit
{
public:

	WT_CLASS( CDspFFTUnit, CDspUnit );

	CDspFFTUnit();
	virtual ~CDspFFTUnit();

	virtual void				SetActive( bool Active );
	virtual void				SetPriority( int32 Priority );

	virtual bool				GetActive() const;
	virtual int32				GetPriority() const;

	virtual int32				GetSpectrumBinCount();
	virtual const float32*		GetSpectrum();

private:
	FSOUND_DSPUNIT*				m_pDspUnit;
};

}	// namespace Audio
}	// namespace WT

