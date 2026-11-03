/***************************************************************************************
 *
 * DspVocalEliminatorUnit.h
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



namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CDspVocalEliminatorUnit> HDspVocalEliminatorUnit;
class CDspVocalEliminatorUnit : public CDspUnit
{
public:
   
	WT_CLASS_NO_CREATOR( CDspVocalEliminatorUnit, CDspUnit );

    CDspVocalEliminatorUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD );
	virtual ~CDspVocalEliminatorUnit();

	virtual void						SetActive( bool Active );
	virtual void						SetPriority( int32 Priority );

	virtual bool						GetActive() const;
	virtual int32						GetPriority() const;

	virtual class CVocalEliminator*	GetVocalEliminator()	{ return m_pVocalElim; }

private:

	CDspCustomUnit*			m_pCustomDspUnit;
	class CVocalEliminator*	m_pVocalElim;
};

}	// namespace Audio
}	// namespace WT

