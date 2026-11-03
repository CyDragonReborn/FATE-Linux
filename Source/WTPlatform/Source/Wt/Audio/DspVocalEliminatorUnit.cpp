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
 * 2/19/2004: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/System/SystemException.h"
#include "WT/Audio/DSPFilter.h"
#include "WT/Audio/DspVocalEliminatorUnit.h"

namespace WT
{
namespace Audio
{

///////////////////////////////////////////////////////////
// code pulled from Boost's Chimera Xbox XbAudioStream.cpp
// originally written by Gary Hinger.
// Modified to remove Xbox references: by Jason Glover
///////////////////////////////////////////////////////////

//Vocal eliminator class
class CVocalEliminator
{
public:
    CVocalEliminator();
    void* DSPCallback_Int16StereoSample(void* newbuffer, int length);
    void* DSPCallback_Float32StereoSample(void* newbuffer, int length);

private:
    static float32 const ms_BassBoost;
    static float32 const ms_VolumeFactor;
    static float32 const ms_MinVolumeFactor;
    static float32 const ms_MaxVolumeFactor;
    static float32 const ms_VolumeCatchUpFactor;

    CDSPFilterIIR m_LPF; //Low pass filter
    CDSPFilterIIR m_HPF; //High pass filter
    float32 m_NormalizationFactor; //Normalization factor
};

//For vocal elimination, don't reach the original song volume exactly - 
//seek quieter output since presumably vocals have been eliminated
float32 const CVocalEliminator::ms_VolumeFactor = 0.93f;

//Boost bass slightly
float32 const CVocalEliminator::ms_BassBoost = 1.21f;

//Don't go beyond -25% or +15% volume
//(Too large a volume leads to too much saturation.)
float32 const CVocalEliminator::ms_MinVolumeFactor = 0.75f;
float32 const CVocalEliminator::ms_MaxVolumeFactor = 1.15f;

float32 const CVocalEliminator::ms_VolumeCatchUpFactor = 0.009f;

// -----------------------------------------------------------------
// Vocal elimination DSP and member functions
// -----------------------------------------------------------------
CVocalEliminator::CVocalEliminator() :
    m_NormalizationFactor(ms_VolumeFactor)
{
    //Coefficients for lowpass IIR filter. This filter has a cutoff frequency of 100 Hz
    //and is used in conjunction with vocal elimination. Note: instead of computing
    //channel - lowpass(channel) as we do below, we could just compute hipass(channel)
    //using a highpass filter with the same cutoff. However, this seems to sound worse.
    static float32 const LPFData[] =
    {
        0.0003745184f,
        0.0076657441f, -0.9923342466f, 2.0000000000f, 1.0000000000f,
        -1.9974911213f, 0.9977065921f, -1.9991978407f, 1.0000000000f,
        -1.9906688929f, 0.9908073545f, -1.9996193647f, 1.0000000000f,
    };

    //Coefficients for hipass IIR filter
    static float32 const HPFData[] =
    {
    /* 6 KHz cutoff:*/
        0.2893008888f,
        -1.0849616528f, 0.0849616900f, -2.0000000000f, 1.0000000000f,
        -1.2729901075f, 0.8907229900f, -1.8004914522f, 1.0000000000f,
        -0.8042849898f, 0.5029172897f, -1.6014521122f, 1.0000000000f,
    };

    m_LPF.SetCoefficients(LPFData, sizeof LPFData / sizeof LPFData[0], 3);
    m_HPF.SetCoefficients(HPFData, sizeof HPFData / sizeof HPFData[0], 3);
}

void* CVocalEliminator::DSPCallback_Int16StereoSample(void *newbuffer, int length)
{
    int16* pBuffer = static_cast<int16*>(newbuffer);
	int32 n = length;

    //Adaptive normalization - try to keep output volume the same as
    //input volume.
    float32 InVolume = 0.0f;
    float32 OutVolume = 0.0f;

    while (n--)
    {
        int32 Left = pBuffer[0];
        int32 Right = pBuffer[1];

        //Compute a sum of the input volume (for both channels)
        InVolume += Left * Left + Right * Right;

        //Extract low frequencies from right channel
        float32 LowPassR = m_LPF.filterElement(static_cast<float32>(Right));
        float32 HiPassR = m_HPF.filterElement(static_cast<float32>(Right));

        //Output Left - midpass(Right) for both channels
        float32 fResult = Left - Right + ms_BassBoost * LowPassR + HiPassR;

        //Compute a sum of the output volume (do only one channel)
        OutVolume += fResult * fResult;
        fResult *= m_NormalizationFactor;
		int32 iResult = Float32ToInt32(m_NormalizationFactor * fResult);

		// NOTE: fResult is multiplied 2 times by m_NormalizationFactor.
		// Gary thinks this may be a bug, but it might sound worse if we remove it
		// so I'm just adding a comment here in case we decide to experiment with
		// it.

        if (iResult < -32768) iResult = -32768;
        else if (iResult > 32767) iResult = 32767;
        *pBuffer++ = static_cast<int16>(iResult);
        *pBuffer++ = static_cast<int16>(iResult);
    }

    //Adjust normalization factor 40 times per second
    if (OutVolume)
    {
        //Double the output volume since we only accounted for one channel previously
        OutVolume *= 2.0f;
        float32 NewFactor = InVolume / OutVolume;

        //Clamp volume to reasonable values
        if (NewFactor < ms_MinVolumeFactor)
            NewFactor = ms_MinVolumeFactor;
        else if (NewFactor > ms_MaxVolumeFactor)
            NewFactor = ms_MaxVolumeFactor;

        //For vocal elimination, don't reach the original song volume exactly - 
        //seek quieter output since presumably vocals have been eliminated
        NewFactor *= ms_VolumeFactor;

        //Approach new factor very smoothly - e.g. over a period of a couple seconds
        //The multiplier here is reliable assuming the number of samples processed
        //remains at 1024. (For X times as many samples, use fac' = (1 + fac)^X - 1.)
        m_NormalizationFactor += ms_VolumeCatchUpFactor * (NewFactor - m_NormalizationFactor);
    }

    return newbuffer;
}

void* CVocalEliminator::DSPCallback_Float32StereoSample(void *newbuffer, int length)
{
    float32* pBuffer = static_cast<float32*>(newbuffer);
	int32 n = length;

    //Adaptive normalization - try to keep output volume the same as
    //input volume.
    float32 InVolume = 0.0f;
    float32 OutVolume = 0.0f;

    while (n--)
    {
        //int32 Left = pBuffer[0];
        //int32 Right = pBuffer[1];
        float32 Left = pBuffer[0];
        float32 Right = pBuffer[1];

        //Compute a sum of the input volume (for both channels)
        InVolume += Left * Left + Right * Right;

        //Extract low frequencies from right channel
        float32 LowPassR = m_LPF.filterElement(Right);
        float32 HiPassR = m_HPF.filterElement(Right);

        //Output Left - midpass(Right) for both channels
        float32 fResult = Left - Right + ms_BassBoost * LowPassR + HiPassR;

        //Compute a sum of the output volume (do only one channel)
        OutVolume += fResult * fResult;
        fResult *= m_NormalizationFactor;
        //int32 iResult = Float32ToInt32(m_NormalizationFactor * fResult);
		fResult *= m_NormalizationFactor;

		// NOTE: fResult is multiplied 2 times by m_NormalizationFactor.
		// Gary thinks this may be a bug, but it might sound worse if we remove it
		// so I'm just adding a comment here in case we decide to experiment with
		// it.


        //if (iResult < -32768) iResult = -32768;
        //else if (iResult > 32767) iResult = 32767;
        *pBuffer++ = fResult;	//static_cast<short>(iResult);
        *pBuffer++ = fResult;	//static_cast<short>(iResult);
    }

    //Adjust normalization factor 40 times per second
    if (OutVolume)
    {
        //Double the output volume since we only accounted for one channel previously
        OutVolume *= 2.0f;
        float32 NewFactor = InVolume / OutVolume;

        //Clamp volume to reasonable values
        if (NewFactor < ms_MinVolumeFactor)
            NewFactor = ms_MinVolumeFactor;
        else if (NewFactor > ms_MaxVolumeFactor)
            NewFactor = ms_MaxVolumeFactor;

        //For vocal elimination, don't reach the original song volume exactly - 
        //seek quieter output since presumably vocals have been eliminated
        NewFactor *= ms_VolumeFactor;

        //Approach new factor very smoothly - e.g. over a period of a couple seconds
        //The multiplier here is reliable assuming the number of samples processed
        //remains at 1024. (For X times as many samples, use fac' = (1 + fac)^X - 1.)
        m_NormalizationFactor += ms_VolumeCatchUpFactor * (NewFactor - m_NormalizationFactor);
    }

    return newbuffer;
}









////////////////////////////////////////////////////////////

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspVocalEliminatorUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspVocalEliminatorUnit* pDspUnit = reinterpret_cast<CDspVocalEliminatorUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CVocalEliminator* pVE = pDspUnit->GetVocalEliminator();
		if( pVE != NULL )
		{
			return pVE->DSPCallback_Int16StereoSample( NewBuffer, Length );
		}
	}
    return NewBuffer;
}

void* __cdecl DspVocalEliminatorUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspVocalEliminatorUnit* pDspUnit = reinterpret_cast<CDspVocalEliminatorUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CVocalEliminator* pVE = pDspUnit->GetVocalEliminator();
		if( pVE != NULL )
		{
			return pVE->DSPCallback_Float32StereoSample( NewBuffer, Length );
		}
	}
    return NewBuffer;
}

#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable: 4311)
CDspVocalEliminatorUnit::CDspVocalEliminatorUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD ) :
	m_pVocalElim( NULL )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	if( m_pVocalElim == NULL )
	{
		m_pVocalElim = new CVocalEliminator();
	}

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspVocalEliminatorUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspVocalEliminatorUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
												cb,
												Priority,
												reinterpret_cast<int32>(this),
												pStreamOrCD );
}
#pragma warning (pop)

///
CDspVocalEliminatorUnit::~CDspVocalEliminatorUnit()
{
	if( m_pVocalElim )
	{
		delete m_pVocalElim;
	}
}

///
void CDspVocalEliminatorUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspVocalEliminatorUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspVocalEliminatorUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspVocalEliminatorUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

}	// namespace Audio
}	// namespace WT

