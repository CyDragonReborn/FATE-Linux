/***************************************************************************************
 *
 * DspGraphicEQUnit.h
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

typedef System::SmartPointer<class CDspGraphicsEQUnit> HDspGraphicEQUnit;
class CDspGraphicEQUnit : public CDspUnit
{
public:

    WT_CLASS_NO_CREATOR( CDspGraphicEQUnit, CDspUnit );

    CDspGraphicEQUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspGraphicEQUnit();

	virtual void						SetActive( bool Active );
	virtual void						SetPriority( int32 Priority );

	virtual bool						GetActive() const;
	virtual int32						GetPriority() const;

	virtual void						SetEqualizerSetting(int32 Index, float32 val);

	virtual int32						GetNumBands( ) const;
	virtual void						GetEqualizerSetting(int32 Index, float32& val) const;

	virtual CGraphicEqualizerFilter*	GetGraphicEQFilterLeft() { return &m_GraphicEQFilterLeft; }
	virtual CGraphicEqualizerFilter*	GetGraphicEQFilterRight() { return &m_GraphicEQFilterRight; }

private:

	CDspCustomUnit*			m_pCustomDspUnit;
	CGraphicEqualizerFilter	m_GraphicEQFilterLeft;
	CGraphicEqualizerFilter	m_GraphicEQFilterRight;
};

}	// namespace Audio
}	// namespace WT

