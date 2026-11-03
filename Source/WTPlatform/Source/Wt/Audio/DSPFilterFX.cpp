/***************************************************************************************
 *
 * DSPFilterFX.cpp
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 4/16/2003: Gary H:	Creation
 * 2/19/2004: Jason Glover: Updated to wtKernel_v2 and added to wtAudio.
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Math/Math.h"
#include "WT/Audio/DSPFilterFX.h"

#pragma warning (push)
#pragma warning (disable: 4201)

#pragma warning (pop)

namespace WT
{
	using namespace Math;

namespace Audio
{

//namespace
//{
//    float32 const g_DefaultSampleRate = 44100.0f;
//} //anonymous namespace

static const float32 KDefaultDspFilterSampleRate = 44100.0f;

#pragma warning (push)
#pragma warning (disable: 4100)

///Prewarp coefficents of a biquad numerator or denominator.
//void Prewarp(float32& w0, float32& w1, float32& w2, float32 fc, float32 fs)
// w0 wasn't used, so I removed it from the params
void Prewarp(float32& w1, float32& w2, float32 fc, float32 fs)
{
	float32 wp = 2.0f * fs * tanf( KPi * fc / fs );
	assert(wp != 0.0);

	w1 /= wp;
	w2 /= wp * wp;
}

#pragma warning (pop)

///Bilinear
///Compute the bilinear tranform of the numerator and
///denominator coefficients and return four z transform
///coeffiecients for Direct Form II realization using
///the IIR filter program.
void Bilinear(
    float32 a0, float32 a1, float32 a2, //Numerator coefficients
    float32 b0, float32 b1, float32 b2, //Denominator coefficients
	float32& k, //Overall gain factor
	float32 fs, //Sampling rate
	float *coef //4 IIR coefficients
    )
{
	assert(coef != NULL);

	// Alpha denominator
	float32 ad = 4.0f * a2 * fs * fs + 2.0f * a1 * fs + a0;

	// Beta denominator
	float32 bd = 4.0f * b2 * fs * fs + 2.0f * b1 * fs + b0;

	// Update gain constant for this section
    assert(bd != 0.0f);

	k *= ad / bd;

	// Bilinear transform the coefficients

	// Beta1
	coef[0] = (2.0f * b0 - 8.0f * b2 * fs * fs) / bd;
	// Beta2
	coef[1] = (4.0f * b2 * fs * fs - 2.0f * b1 * fs + b0) / bd;
	// Alpha1
	coef[2] = (2.0f * a0 - 8.0f * a2 * fs * fs) / ad;
	// Alpha2
	coef[3] = (4.0f * a2 * fs * fs - 2.0f * a1 * fs + a0) / ad;
}

///Design IIR filter coefficients from analog prototype coefficients and determine 
//magnitude response. This function calculates the coefficients for a low-pass 3 
//section (5th order) elliptic filter with 0.28 dB passband ripple and 40 dB 
//stopband attenuation.

void IIRDesign_LPF(float32 SampleRate, float32 Frequency, float32 CoefArray[13])
{
    //Note: these magic numbers come from the "C++ Algorithms for Digital Signal Processing" book
    int32 const Sections = 3;
    static float32 const SectionData[Sections][6] =
    {
        0.540100f, 0.0f, 0.0f, 0.540100f, 1.0f, 0.0f,
        1.0626604f, 0.0f, 0.268855f, 1.0626604f, 0.16116f, 1.0f,
        0.685228f, 0.0f, 0.36546f, 0.685228f, 0.64820f, 1.0f,
    };

    assert(CoefArray != NULL);

    float32 fs = 1.0f; //Sampling rate

    if (SampleRate == 0.0f)
    {
		WT_THROW( System::CArgumentOutOfRangeException, "SampleRate" );
    }

    float32 fc = Frequency / SampleRate; //Cutoff frequency

    //Don't allow cutoff frequencies above the Nyquist frequency
    if (fc > 0.5f)
    {
		WT_THROW( System::CArgumentOutOfRangeException, "Frequency exceeds Nyquist frequency" );
    }

    //int32 const Length = 1 + (Sections << 2);
    float32 *pCoef = CoefArray + 1;

	// Start gain at 1.0
	float32 k = 1.0;

	for (int i = 0; i < Sections; i++)
	{
        //Numerator coefficients
        float32 a0 = SectionData[i][0];
        float32 a1 = SectionData[i][1];
        float32 a2 = SectionData[i][2];
		//Prewarp( a0, a1, a2, fc, fs );
		Prewarp( a1, a2, fc, fs );

        //Denominator coefficients
        float32 b0 = SectionData[i][3];
        float32 b1 = SectionData[i][4];
        float32 b2 = SectionData[i][5];
		//Prewarp( b0, b1, b2, fc, fs );
		Prewarp( b1, b2, fc, fs );

		Bilinear( a0, a1, a2, b0, b1, b2, k, fs, pCoef );
		pCoef += 4;
	}

	// Store gain constant
	CoefArray[0] = k;
    return;
}

CAudioWaveSine::CAudioWaveSine()
{
    m_SampleRate = KDefaultDspFilterSampleRate; //g_DefaultSampleRate;
    m_C = 1.0f;
    m_S = 0.0f;
    SetFrequency(300.0f);
}

float32 CAudioWaveSine::GetFrequency() const
{
    return m_Frequency;
}

void CAudioWaveSine::SetFrequency(float32 Frequency)
{
    m_Frequency = Frequency;
    UpdateDeltas();
}

void CAudioWaveSine::UpdateDeltas()
{
    float32 Delta = (KTwoPi / m_SampleRate) * m_Frequency;
    m_CosDelta = Math_Cos(Delta);
    m_SinDelta = Math_Sin(Delta);

    //When frequency == 0, use a constant signal of 1.0
    if (m_Frequency == 0.0f)
    {
        m_C = 1.0f;
        m_S = 0.0f;
    }
}

void CAudioWaveSine::SetSampleRate(float32 SampleRate)
{
    m_SampleRate = SampleRate;
    UpdateDeltas();
}

void CAudioWaveSine::Advance()
{
    float32 C = m_C;
    m_C = C * m_CosDelta - m_S * m_SinDelta;
    m_S = C * m_SinDelta + m_S * m_CosDelta;

    ++m_NumIter;
    if (!(m_NumIter & 1023)) //Renormalize c, s values every 1023 iterations
    {
        float m = Math_Hypot(m_C, m_S);
        if (m)
        {
            m = 1.0f / m;
            m_C *= m;
            m_S *= m;
        } else {
            m_C = 1.0f;
            m_S = 0.0f;
        }
    }
}

CGraphicEqualizerFilter::CGraphicEqualizerFilter()
{
    //The cutoff frequency is sqrt(2) times the center frequency of the band
    float32 Freq = 31.25f * Math_Sqrt(2.0f);
    for (int32 i = 0; i < eNumBands - 1; ++i)
    {
        float32 Coef[13];
        IIRDesign_LPF(KDefaultDspFilterSampleRate, Freq, Coef);
        m_IIRLPFs[i].SetCoefficients(Coef, sizeof Coef / sizeof *Coef, 3);
        Freq *= 2.0f;
    }
    std::fill(m_EqualizerSettings, m_EqualizerSettings + eNumBands, 1.0f);
}

float32 CGraphicEqualizerFilter::Filter(float32 In)
{
    float32 Out = In; // * m_EqualizerSettings[eNumBands - 1];
    for (int i = 0; i < eNumBands - 1; i++)
    {
        float32 Factor = m_EqualizerSettings[i] - m_EqualizerSettings[i + 1];
        
        if (Factor == 0.0f) continue;

        Out += Factor * reinterpret_cast<CDSPFilterIIR&>(
            m_IIRLPFs[i]).filterElement(In);
    }
    
    return Out;
}

void CGraphicEqualizerFilter::GetEqualizerSetting(int32 Index, float32& val) const
{
	if( Index < 0 || Index <= eNumBands )
	{
		WT_THROW( System::CArgumentOutOfRangeException, "Index" );
	}

	val = m_EqualizerSettings[Index];
}

void CGraphicEqualizerFilter::SetEqualizerSetting(int32 Index, float32 val)
{
	if( Index < 0 || Index <= eNumBands )
	{
		WT_THROW( System::CArgumentOutOfRangeException, "Index" );
	}

	m_EqualizerSettings[Index] = val;
}

bool CGraphicEqualizerFilter::IsNormal() const
{
    for (int32 i = 0; i < eNumBands; ++i)
        if (m_EqualizerSettings[i] != 1.0f)
            return false;

    return true;
}

CAudioFilter::CAudioFilter() :
	m_SampleRate( KDefaultDspFilterSampleRate ),
    m_Magnitude( 0.5f )
{
}

CAudioFilter::~CAudioFilter()
{
}

float32 CAudioFilter::GetSampleRate() const
{
    return m_SampleRate;
}

float32 CAudioFilter::GetMagnitude() const 
{
    return m_Magnitude;
}

void CAudioFilter::SetSampleRate(float32 SampleRate)
{
    m_SampleRate = SampleRate;
    OnSampleRateChanged();
    return;
}

void CAudioFilter::OnSampleRateChanged()
{
}

void CAudioFilter::OnMagnitudeChanged()
{
}

void CAudioFilter::SetMagnitude(float32 Magnitude)
{
    m_Magnitude = Magnitude;
    OnMagnitudeChanged();
    return;
}

float32 CAudioFilterNull::Filter(float32 In)
{
    return In;
}

CAudioFilterHistory::CAudioFilterHistory()
{
    m_BufferedTime = 1.0f;
    UpdateBufferSize();
    m_DelayTime = 0.125f;
    UpdateDelaySamples();
    m_Next = 0;
    m_Magnitude = 0.5f;
}

void CAudioFilterHistory::OnSampleRateChanged()
{
    CAudioFilter::OnSampleRateChanged();
    UpdateBufferSize();
    UpdateDelaySamples();
}

float32 CAudioFilterHistory::GetBufferedTime() const
{
    return m_BufferedTime;
}

float32 CAudioFilterHistory::GetDelayTime() const
{
    return m_DelayTime;
}

void CAudioFilterHistory::SetBufferedTime(float32 BufferedTime)
{
    if (BufferedTime < 0.0f)
    {
		WT_THROW( System::CArgumentOutOfRangeException, "BufferedTime" );
    }

    m_BufferedTime = BufferedTime;
    UpdateBufferSize();
}

void CAudioFilterHistory::SetDelayTime(float32 DelayTime)
{
    if (DelayTime < 0.0f || DelayTime > m_BufferedTime)
    {
        WT_THROW( System::CArgumentOutOfRangeException, "DelayTime" );
    }

    m_DelayTime = DelayTime;
    UpdateDelaySamples();
}

void CAudioFilterHistory::UpdateBufferSize()
{
	int32 const NewSize = Float32ToInt32(Math_Ceil(m_BufferedTime * m_SampleRate));

    if (static_cast<int32>(m_Buf.size()) != NewSize)
    {
        m_Buf.clear();
        m_Buf.resize(NewSize, 0.0f);
    }
}

void CAudioFilterHistory::UpdateDelaySamples()
{
    m_DelaySamples = Float32ToInt32(m_DelayTime * m_SampleRate);
    if (m_DelaySamples < 1)
    {
        m_DelaySamples = 1;
    } else if (m_DelaySamples > static_cast<int32>(m_Buf.size())) {
        m_DelaySamples = static_cast<int32>(m_Buf.size());
    }
}

float32 CAudioFilterChorus::Filter(float32 In)
{
    int32 old = m_Next - m_DelaySamples;

    if (old < 0)
        old += static_cast<int32>(m_Buf.size());

    float32 Result = In + m_Magnitude * m_Buf[old];

    m_Buf[m_Next] = In; //No feedback
    if (++m_Next >= static_cast<int32>(m_Buf.size()))
        m_Next = 0;

    return Result;
}

CAudioFilterFlange::CAudioFilterFlange()
{
    //Set min and max delay times in ms
    m_MinDelayTime = 0.000f;
    UpdateMinDelaySamples();
    m_MaxDelayTime = 0.005f;
    UpdateMaxDelaySamples();

    //Pick an arbitrary frequency
    SetFrequency(0.15f);
}

float32 CAudioFilterFlange::GetMinDelayTime() const
{
    return m_MinDelayTime;
}

float32 CAudioFilterFlange::GetMaxDelayTime() const
{
    return m_MaxDelayTime;
}

float32 CAudioFilterFlange::GetFrequency() const
{
    return m_Wave.GetFrequency();
}

void CAudioFilterFlange::SetMinDelayTime(float32 MinDelayTime)
{
    if (MinDelayTime < 0.0f || MinDelayTime > m_BufferedTime)
    {
        WT_THROW( System::CArgumentOutOfRangeException, "MinDelayTime" );
    }

    m_MinDelayTime = MinDelayTime;
    UpdateMinDelaySamples();
}

void CAudioFilterFlange::SetMaxDelayTime(float32 MaxDelayTime)
{
    if (MaxDelayTime < 0.0f || MaxDelayTime > m_BufferedTime)
    {
        WT_THROW( System::CArgumentOutOfRangeException, "MaxDelayTime" );
    }

    m_MaxDelayTime = MaxDelayTime;
    UpdateMaxDelaySamples();
}

void CAudioFilterFlange::SetFrequency(float32 Frequency)
{
    m_Wave.SetFrequency(Frequency);
}

void CAudioFilterFlange::OnSampleRateChanged()
{
    CAudioFilter::OnSampleRateChanged();
    UpdateMinDelaySamples();
    UpdateMaxDelaySamples();
    m_Wave.SetSampleRate(m_SampleRate);
}

void CAudioFilterFlange::UpdateMinDelaySamples()
{
    m_MinDelaySamples = m_MinDelayTime * m_SampleRate;
    if (m_MinDelaySamples < 1.0f)
    {
        m_MinDelaySamples = 1.0f;
    } else if (m_MinDelaySamples > m_Buf.size()) {
        m_MinDelaySamples = static_cast<float32>(m_Buf.size());
    }

    m_MidDelaySamples = 0.5f * (m_MaxDelaySamples + m_MinDelaySamples);
    m_DeltaDelaySamples = 0.5f * (m_MaxDelaySamples - m_MinDelaySamples);
}

void CAudioFilterFlange::UpdateMaxDelaySamples()
{
    m_MaxDelaySamples = m_MaxDelayTime * m_SampleRate;
    if (m_MaxDelaySamples < 1.0f)
    {
        m_MaxDelaySamples = 1.0f;
    } else if (m_MaxDelaySamples > m_Buf.size()) {
        m_MaxDelaySamples = static_cast<float32>(m_Buf.size());
    }

    m_MidDelaySamples = 0.5f * (m_MaxDelaySamples + m_MinDelaySamples);
    m_DeltaDelaySamples = 0.5f * (m_MaxDelaySamples - m_MinDelaySamples);
}

float32 CAudioFilterFlange::Filter(float32 In)
{
    float32 FDelay = m_MidDelaySamples + m_DeltaDelaySamples * m_Wave.GetSin();
    m_Wave.Advance();

    //Get a historical value for a floating point index
    //Find two adjacent historical positions
    m_DelaySamples = Float32ToInt32(Math_Floor(FDelay));
    float32 Frac = FDelay - m_DelaySamples;

    int32 old1 = m_Next - m_DelaySamples;
    if (old1 < 0)
        old1 += static_cast<int32>(m_Buf.size());
    int32 old2 = old1 - 1;
    if (old2 < 0)
        old2 += static_cast<int32>(m_Buf.size());

    //Linearly interpolate between two historical values
    float32 val1 = m_Buf[old1];
    float32 val2 = m_Buf[old2];
    float32 Result = In + m_Magnitude * (val1 + Frac * (val2 - val1));

    m_Buf[m_Next] = Result; //No feedback
    if (++m_Next >= static_cast<int32>(m_Buf.size()))
        m_Next = 0;

    return Result;
}

float32 CAudioFilterDelay::Filter(float32 In)
{
    int32 old = m_Next - m_DelaySamples;

    if (old < 0)
        old += static_cast<int32>(m_Buf.size());

    float32 Result = In + m_Magnitude * m_Buf[old];

    m_Buf[m_Next] = Result;
    if (++m_Next >= static_cast<int32>(m_Buf.size()))
        m_Next = 0;

    return Result;
}

//Reverberation
//These are fractions of a second
float32 const CAudioFilterReverberation::ms_Delays[eNumDelays] =
{
    0.034875f,
    0.043625f,
    0.073375f,
    0.110125f,
    0.155625f,
    0.187125f,
};

float32 const CAudioFilterReverberation::ms_Decays[eNumDelays] =
{
    //Note: for the reverb effect to be stable, the sum of these must not exceed 1
    0.28f,
    0.21f,
    0.15f,
    0.09f,
    0.05f,
    0.03f,
};

CAudioFilterReverberation::CAudioFilterReverberation()
{
    SetDelayFactor(1.0f);
}

float32 CAudioFilterReverberation::GetDelayFactor() const
{
    return m_DelayFactor;
}

void CAudioFilterReverberation::SetDelayFactor(float32 DelayFactor)
{
    if (DelayFactor < 0.1f || DelayFactor > 1.0f)
    {
        WT_THROW( System::CArgumentOutOfRangeException, "DelayFactor" );
    }

    m_DelayFactor = DelayFactor;
    UpdateDelays();
    return;
}

void CAudioFilterReverberation::OnSampleRateChanged()
{
    CAudioFilter::OnSampleRateChanged();
    UpdateDelays();
}

void CAudioFilterReverberation::UpdateDelays()
{
    float32 DelayFactor = m_DelayFactor * m_SampleRate;

    for (int32 i = 0; i < eNumDelays; ++i)
    {
        m_Delays[i] = Float32ToInt32(ms_Delays[i] * DelayFactor);

        if (m_Delays[i] < 1)
            m_Delays[i] = 1;

        if (m_Delays[i] > static_cast<int32>(m_Buf.size()))
            m_Delays[i] = static_cast<int32>(m_Buf.size());
    }
}

float32 CAudioFilterReverberation::Filter(float32 In)
{
    float32 Result;

    if (m_Magnitude == 0.0f)
    {
        Result = In;
    } else {
        Result = 0.0f;

        for (int32 i = 0; i < eNumDelays; ++i)
        {
            int32 old = m_Next - m_Delays[i];
            if (old < 0)
                old += static_cast<int32>(m_Buf.size());

            Result += ms_Decays[i] * m_Buf[old];
        }
        
        Result = In + m_Magnitude * Result;
    }

    m_Buf[m_Next] = Result;
    if (++m_Next >= static_cast<int32>(m_Buf.size()))
        m_Next = 0;

    return Result;
}

CAudioFilterPitchShift::CAudioFilterPitchShift()
{
    m_Next = 0;
    SetLatency(0.008f);
    SetPitchFactor(1.0f);
}

float32 CAudioFilterPitchShift::GetPitchFactor() const
{
    return m_PitchFactor;
}

float32 CAudioFilterPitchShift::GetLatency() const
{
    return m_Latency;
}

void CAudioFilterPitchShift::SetLatency(float32 Latency)
{
    if (Latency <= 0.0f)
	{
        WT_THROW( System::CArgumentOutOfRangeException, "Latency" );
	}

    m_Latency = Latency;
    UpdateSweepMax();
}

void CAudioFilterPitchShift::UpdateSweepMax()
{
    m_SweepMax = m_Latency * m_SampleRate;
    m_SweepPos = 0.0f;

    m_Buf.resize(Float32ToInt32(Math_Ceil(m_SweepMax)), 0.0f);
}

void CAudioFilterPitchShift::OnSampleRateChanged()
{
    CAudioFilter::OnSampleRateChanged();
    UpdateSweepMax();
}

void CAudioFilterPitchShift::SetPitchFactor(float32 PitchFactor)
{
    if (PitchFactor < 0.2f || PitchFactor > 5.0f)
    {
        WT_THROW( System::CArgumentOutOfRangeException, "PitchFactor" );
    }

    m_PitchFactor = PitchFactor;
    m_SweepSpeed = 1.0f - PitchFactor;

    //If disabling pitch shifting, then reset position
    if (m_SweepSpeed == 0.0f)
    {
        m_SweepPos = 0.0f;
    }
}

float32 CAudioFilterPitchShift::Filter(float32 In)
{
    m_Buf[m_Next] = In;

    if (m_Magnitude == 0.0f || m_PitchFactor == 1.0f)
    {
        if (++m_Next >= static_cast<int32>(m_Buf.size()))
            m_Next = 0;
        return In;
    }

    float32 Sweep1 = m_Next - m_SweepPos;
    float32 Sweep2 = m_Next - m_SweepMax + m_SweepPos;

    float32 Sweep1Val;
    float32 Sweep2Val;
    int32 i;
    float32 Frac, val1, val2;

    i = Float32ToInt32(Math_Floor(Sweep1));
    Frac = Sweep1 - i;
    if (i < 0)
    {
        i += static_cast<int32>(m_Buf.size());
    }
    val1 = m_Buf[i];
    if (++i >= static_cast<int32>(m_Buf.size()))
    {
        i = 0;
    }
    val2 = m_Buf[i];
    Sweep1Val = val1 + Frac * (val2 - val1);

    i = Float32ToInt32(Math_Floor(Sweep2));
    Frac = Sweep2 - i;
    if (i < 0)
    {
        i += static_cast<int32>(m_Buf.size());
    }
    val1 = m_Buf[i];
    if (++i >= static_cast<int32>(m_Buf.size()))
    {
        i = 0;
    }
    val2 = m_Buf[i];
    Sweep2Val = val1 + Frac * (val2 - val1);

    //As m_SweepPos approaches m_SweepMax, Result should
    //approach 1.
    float32 Result = 0.0f;
    if (m_SweepPos > 0.85f * m_SweepMax)
    {
        float32 x = (m_SweepPos / m_SweepMax - 0.85f) * (1.0f / 0.15f);
        //Compute 3x^2 - 2x^3
        //This is an approximation to the "smoothstep" function. This is
        //the lowest degree polynomial p(x) satisfying p(0) = 0, p(1) = 1, 
        //and p'(0) = p'(1) = 0.
        Result = x * (3.0f * x - 2.0f * x * x);
    }

    if (m_SweepSpeed < 0.0f) Result = 1.0f - Result;

    Result = Sweep1Val + Result * (Sweep2Val - Sweep1Val);

    if (++m_Next >= static_cast<int32>(m_Buf.size()))
        m_Next = 0;

    m_SweepPos += Math_Fabs(m_SweepSpeed);
    if (m_SweepPos > m_SweepMax)
        m_SweepPos -= m_SweepMax;

    return In + m_Magnitude * (Result - In);
}

CAudioFilterRingModulator::CAudioFilterRingModulator()
{
}

void CAudioFilterRingModulator::OnSampleRateChanged()
{
    CAudioFilter::OnSampleRateChanged();
    m_Wave.SetSampleRate(m_SampleRate);
}

float32 CAudioFilterRingModulator::GetFrequency() const
{
    return m_Wave.GetFrequency();
}

void CAudioFilterRingModulator::SetFrequency(float32 Frequency)
{
    m_Wave.SetFrequency(Frequency);
}

float32 CAudioFilterRingModulator::Filter(float32 In)
{
    float32 Result = In * m_Wave.GetCos();
    m_Wave.Advance();
    return In + m_Magnitude * (Result - In);
}

CAudioFilterDistortion::CAudioFilterDistortion()
{
    SetDistortion(0.0f);
}

float32 CAudioFilterDistortion::GetDistortion() const
{
    return m_Distortion;
}

void CAudioFilterDistortion::SetDistortion(float32 Distortion)
{
    if (Distortion < 0.0f || Distortion > 1.0f)
    {
		WT_THROW( System::CArgumentOutOfRangeException, "Distortion" ); 
	}

    m_Distortion = Distortion;
    m_Exponent = 1.0f - Distortion;
}

float32 CAudioFilterDistortion::Filter(float32 In)
{
    float32 Result;

    Result = 32767.0f * Math_Pow(Math_Fabs(In) * 
        (1.0f / 32768.0f), m_Exponent);

    if (In < 0)
        Result = -Result;

    return In + m_Magnitude * (In - Result);
}

CAudioFilterPass::CAudioFilterPass()
{
    m_CutoffFreq = 0.0f;
    SetCutoffFrequency(2000.0f);
}

float32 CAudioFilterPass::GetCutoffFrequency() const
{
    return m_CutoffFreq;
}

void CAudioFilterPass::SetCutoffFrequency(float32 CutoffFreq)
{
	if (CutoffFreq <= 0.0f)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "CutoffFreq" );
	}

    if (CutoffFreq == m_CutoffFreq)
    {
        //If setting same frequency, then do nothing
        return;
    }

    float32 Coef[13];
    IIRDesign_LPF(m_SampleRate, CutoffFreq, Coef);
    m_CutoffFreq = CutoffFreq;
    m_IIRLPF.SetCoefficients(Coef, sizeof Coef / sizeof *Coef, 3);
}

void CAudioFilterPass::OnSampleRateChanged()
{
    SetCutoffFrequency(m_CutoffFreq);
}

float32 CAudioFilterLowPass::Filter(float32 In)
{
    if (m_Magnitude == 0.0f)
        return In;
    else
        return In + m_Magnitude * (m_IIRLPF.filterElement(In) - In);
}

float32 CAudioFilterHighPass::Filter(float32 In)
{
    if (m_Magnitude == 0.0f)
        return In;
    else
        return In - m_Magnitude * m_IIRLPF.filterElement(In);
}

CAudioFilterScratch::CAudioFilterScratch()
{
    m_ScratchSampleRate = KDefaultDspFilterSampleRate;
    UpdateSampleDelta();
    m_SweepPos = 0.0f;
    m_InputVolume = 1.0f;
}

int32 CAudioFilterScratch::GetScratchSize() const
{
    return static_cast<int32>(m_Scratch.size());
}

void CAudioFilterScratch::SetScratchSize(int32 Size)
{
    m_Scratch.resize(Size);
    Stop();
}

float32* CAudioFilterScratch::GetScratchBuffer()
{
    return m_Scratch.empty() ? NULL : &m_Scratch[0];
}

float32 CAudioFilterScratch::GetInputVolume() const
{
    return m_InputVolume;
}

void CAudioFilterScratch::Play()
{
    //If still playing, then fail
    if (m_SweepPos < static_cast<float32>(static_cast<int32>(m_Scratch.size()) - 1))
    {
		WT_THROW( CAudioException, "already playing" );
    }

    m_SweepPos = 0.0f;
}

bool CAudioFilterScratch::IsPlaying() const
{
    return m_SweepPos < static_cast<float32>(static_cast<int32>(m_Scratch.size()) - 1) ? true : false;
}

void CAudioFilterScratch::Stop()
{
    m_SweepPos = static_cast<float32>(static_cast<int32>(m_Scratch.size()) - 1);
    if (m_SweepPos < 0.0f)
    {
        m_SweepPos = 0.0f;
    }
}

void CAudioFilterScratch::SetInputVolume(float32 InputVolume)
{
    m_InputVolume = InputVolume;

    return;
}

void CAudioFilterScratch::OnSampleRateChanged()
{
    CAudioFilter::OnSampleRateChanged();
    UpdateSampleDelta();
}

void CAudioFilterScratch::SetScratchSampleRate(float32 ScratchSampleRate)
{
    m_ScratchSampleRate = ScratchSampleRate;
    UpdateSampleDelta();
}

void CAudioFilterScratch::UpdateSampleDelta()
{
    m_SampleDelta = m_ScratchSampleRate / m_SampleRate;    
}

float32 CAudioFilterScratch::Filter(float32 In)
{
    int32 ScratchSize = static_cast<int32>(m_Scratch.size());
    if (!ScratchSize || m_SampleDelta == 0.0f)
    {
        return In;
    }

    m_SweepPos += m_SampleDelta;
    if (m_SweepPos > ScratchSize - 1) 
    {
        m_SweepPos = static_cast<float32>(ScratchSize - 1);
    }

    //If reached end of buffer, return
    if (m_SweepPos >= ScratchSize - 1)
    {
        return In;
    }

    float32 SweepVal;
    int32 i;
    float32 Frac, val1, val2;

    i = Float32ToInt32(Math_Floor(m_SweepPos));
    Frac = m_SweepPos - i;
    val1 = m_Scratch[i];
    if (++i >= ScratchSize)
    {
        val2 = 0.0f;
    } else {
        val2 = m_Scratch[i];
    }
    SweepVal = val1 + Frac * (val2 - val1);

    return m_InputVolume * In + m_Magnitude * SweepVal;
}

CAudioFilterPan::CAudioFilterPan()
{
    m_Channel = 0;
    m_Pan = 0.5f;
    UpdateFactor();
}

int32 CAudioFilterPan::GetChannel() const
{
    return m_Channel;
}

float32 CAudioFilterPan::GetPan() const
{
    return m_Pan;
}

void CAudioFilterPan::SetChannel(int32 Channel)
{
    if (Channel < 0 || Channel > 1)
    {
		WT_THROW( System::CArgumentOutOfRangeException, "Channel" );
    }

    m_Channel = Channel;
    UpdateFactor();
}

void CAudioFilterPan::SetPan(float32 Pan)
{
    if (Pan < 0.0f || Pan > 1.0f)
    {
		WT_THROW( System::CArgumentOutOfRangeException, "Pan" );
    }

    m_Pan = Pan;
    UpdateFactor();
}

void CAudioFilterPan::UpdateFactor()
{
    m_Factor = m_Pan;
    if (m_Channel == 0)
        m_Factor = 1.0f - m_Factor;

    //Divide by the root of the sum of the squares of m_Factor and 1 - m_Factor
    m_Factor /= Math_Sqrt(m_Factor * m_Factor + (1.0f - m_Factor) * (1.0f - m_Factor));

    //Include magnitude
    m_Factor = 1.0f + m_Magnitude * (m_Factor - 1.0f);
}

void CAudioFilterPan::OnMagnitudeChanged()
{
    UpdateFactor();
}

float32 CAudioFilterPan::Filter(float32 In)
{
    return In * m_Factor;
}

}	// namespace Audio
}	// namespace WT
