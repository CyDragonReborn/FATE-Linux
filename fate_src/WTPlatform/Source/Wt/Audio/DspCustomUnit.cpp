/***************************************************************************************
 *
 * DspCustomUnit.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/17/2004: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "windows.h"
#include "WT/Audio/AudioStream.h"
#include "WT/Audio/AudioCD.h"
#include "WT/Audio/DspCustomUnit.h"
#include "WT/Audio/DspEngine.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspUnit)
WT_DEFINE_CLASS(CDspCustomUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
///
void* F_CALLBACKAPI DspCustomUnitCallback(	void *originalbuffer,
													void *newbuffer,
													int length,
													int param )
{
	CDspCustomUnit* pDspUnit = reinterpret_cast<CDspCustomUnit*>(param);
	if( pDspUnit != NULL )
	{
		DspCallback Cb(NULL);
		int32 CbParam(0);
		pDspUnit->GetDspCallback( Cb, CbParam );
		if( Cb != NULL )
		{
			return Cb( originalbuffer, newbuffer, static_cast<int32>(length), CbParam );
		}

	}
	return newbuffer;
}

#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable: 4311)
CDspCustomUnit::CDspCustomUnit( DspCallback Cb, int32 Priority, int32 Param, CObject* pStreamOrCD ) :
	m_pDspUnit( NULL ),
	m_Cb( NULL ),
	m_CbParam( 0 )
{

	if( pStreamOrCD == NULL )
	{
		m_pDspUnit = FSOUND_DSP_Create(	DspCustomUnitCallback,
										static_cast<int>(Priority),
										reinterpret_cast<int>(this) );
		if (m_pDspUnit == NULL)
		{
			WT_THROW( CAudioException, "FSOUND_DSP_Create failure" );
		}
	}
	else
	{
		FSOUND_STREAM* pFS( NULL );

		if( pStreamOrCD->GetType() == CAudioStream::GetClassType() )
		{
			CAudioStream* pAS = static_cast<CAudioStream*>( pStreamOrCD );
			assert( pAS != NULL );
			pFS = pAS->GetStream();
		}
		else if( pStreamOrCD->GetType() == CAudioCD::GetClassType() )
		{
			CAudioCD* pCD = static_cast<CAudioCD*>( pStreamOrCD );
			assert( pCD != NULL );
			pFS = pCD->GetStream();
		}
		else
		{
			assert( 0 );
		}

		if (pFS == NULL)
		{
			WT_THROW( CAudioException, "pStreamOrCD->GetStream() is NULL" );
		}

		m_pDspUnit = FSOUND_Stream_CreateDSP(	pFS,
												DspCustomUnitCallback,
												static_cast<int>(Priority),
												reinterpret_cast<int>(this) );
		if (m_pDspUnit == NULL)
		{
			WT_THROW( CAudioException, "FSOUND_Stream_CreateDSP failure" );
		}
	}

	assert( m_pDspUnit != NULL );

	m_Cb = Cb;
	m_CbParam = Param;
}
#pragma warning (pop)

///
CDspCustomUnit::~CDspCustomUnit()
{
	if( m_pDspUnit != NULL )
	{
		FSOUND_DSP_Free( m_pDspUnit );
	}
}

///
void CDspCustomUnit::SetActive( bool Active )
{
	assert( m_pDspUnit != NULL );
	signed char active((Active == true) ? TRUE : FALSE);
	FSOUND_DSP_SetActive( m_pDspUnit, active );
}

///
void CDspCustomUnit::SetPriority( int32 Priority )
{
	assert( m_pDspUnit != NULL );
    FSOUND_DSP_SetPriority( m_pDspUnit, static_cast<int>(Priority) );
}

///
bool CDspCustomUnit::GetActive() const
{
	assert( m_pDspUnit != NULL );
	return ((FSOUND_DSP_GetActive( m_pDspUnit ) == TRUE) ? true : false);
}

///
int32 CDspCustomUnit::GetPriority() const
{
	assert( m_pDspUnit != NULL );
	return static_cast<int32>(FSOUND_DSP_GetPriority( m_pDspUnit ));
}

void CDspCustomUnit::GetDspCallback( DspCallback& Cb, int32& Param )
{
	//assert( pCb != NULL && pParam != NULL );
	Cb = m_Cb;
	Param = m_CbParam;
}

}	// namespace Audio
}	// namespace WT
