/***************************************************************************************
 *
 * DSPFilterFX.h
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
#pragma once

#include "WT/Audio/DSPFilter.h"

namespace WT
{
namespace Audio
{

//Calculate the coefficients for a 3-section low pass IIR filter with the specified cut-off frequency
void IIRDesign_LPF(float32 SampleRate, float32 Frequency, float32 CoefArray[13]);

//Fast sine wave generator
class CAudioWaveSine
{
public:

    CAudioWaveSine();

    float32 GetFrequency() const;
    float32 GetSin() const {return m_S;}
    float32 GetCos() const {return m_C;}

    void Advance();

    void SetFrequency(float32 Frequency);

    void SetSampleRate(float32 SampleRate);

private:
    float32 m_SampleRate;
    float32 m_Frequency; //Frequency of modulating signal in Hz

    //Cosine and sine of delta phase / sample
    float32 m_CosDelta;
    float32 m_SinDelta;

    //Cosine and sine of phase
    float32 m_C;
    float32 m_S;
    int32 m_NumIter;

    void UpdateDeltas();
};

//////////////////////////////////////////////////////////////////
// 10-band graphic equalizer
//
// 9 low-pass IIR filters support the ten-band graphic equalizer with 
// passbands centered at these frequencies in Hz: 31.25, 62.5, 125, 250, 500,
// 1K, 2K, 4K, 8K, and 16K.
// Note: instead of 31.25 and 62.5, display these "preferred" frequencies
// to the user: 31.5, 63
//////////////////////////////////////////////////////////////////
class CGraphicEqualizerFilter
{
public:
    CGraphicEqualizerFilter();

    float32 Filter(float32 In);

    enum {eNumBands = 10};

    void GetEqualizerSetting(int32 Index, float32& val) const;

    void SetEqualizerSetting(int32 Index, float32 val);

    bool IsNormal() const;

private:
    float32 m_EqualizerSettings[eNumBands];

    CDSPFilterIIR m_IIRLPFs[eNumBands - 1];
};

//A base class for audio filters
class CAudioFilter
{
public:
    CAudioFilter();
    virtual ~CAudioFilter();

    //Get sample rate
    float32 GetSampleRate() const;

    float32 GetMagnitude() const;

    //Set sample rate
    void SetSampleRate(float32 SampleRate);

    ///Set the magnitude of the effect. The meaningful range of magnitude depends on the 
    ///specific effect type. 0.0 through 1.0 is usually valid. One notable exception is 
    ///the AudioFilterDelay effect.
    void SetMagnitude(float32 Magnitude);

    virtual float32 Filter(float32 In) = 0;

protected:
    float32 m_SampleRate;
    float32 m_Magnitude;

protected:
    virtual void OnSampleRateChanged();
    virtual void OnMagnitudeChanged();
};

class CAudioFilterNull : public CAudioFilter
{
    float32 Filter(float32 In);
};

//A DSP effect filter using history data
class CAudioFilterHistory : public CAudioFilter
{
public:
    CAudioFilterHistory();

    float32 GetBufferedTime() const;

    //Get the delay time.
    float32 GetDelayTime() const;

    void SetBufferedTime(float32 BufferedTime);

    //Set the delay time.
    void SetDelayTime(float32 delay);

protected:
    float32 m_BufferedTime;
    float32 m_DelayTime;
    int32 m_DelaySamples;
    int32 m_Next;
    std::vector<float32> m_Buf;

    virtual void OnSampleRateChanged();

private:
    void UpdateBufferSize();

    void UpdateDelaySamples();
};

class CAudioFilterChorus : public CAudioFilterHistory
{
public:
    float32 Filter(float32 In);
};

class CAudioFilterFlange : public CAudioFilterHistory
{
public:
    CAudioFilterFlange();
    float32 Filter(float32 In);

    float32 GetMinDelayTime() const;
    float32 GetMaxDelayTime() const;
    float32 GetFrequency() const;

    void SetMinDelayTime(float32 MinDelayTime);
    void SetMaxDelayTime(float32 MaxDelayTime);
    void SetFrequency(float32 Frequency);

protected:
    virtual void OnSampleRateChanged();

private:
    using CAudioFilterHistory::GetDelayTime;
    using CAudioFilterHistory::SetDelayTime;

    float32 m_MinDelayTime;
    float32 m_MaxDelayTime;
    float32 m_MinDelaySamples;
    float32 m_MaxDelaySamples;
    float32 m_MidDelaySamples;
    float32 m_DeltaDelaySamples;
 
    void UpdateMinDelaySamples();
    void UpdateMaxDelaySamples();

    CAudioWaveSine m_Wave;
};

//The Delay effect works best with a magnitude in the range (-1.0, +1.0).
//Values of absolute magnitude 1.0 or greater can lead to out-of-control results.
//For values less than 0.0, the effect is also known as phasing.
class CAudioFilterDelay : public CAudioFilterHistory
{
public:
    float32 Filter(float32 In);
};

class CAudioFilterReverberation : public CAudioFilterHistory
{
public:
    CAudioFilterReverberation();

    float32 GetDelayFactor() const;
    void SetDelayFactor(float32 DelayFactor);

    float32 Filter(float32 In);

protected:
    virtual void OnSampleRateChanged();

private:
    void UpdateDelays();

    enum {eNumDelays = 6};
    float32 m_DelayFactor;
    int32 m_Delays[eNumDelays];
    static float32 const ms_Delays[eNumDelays];
    static float32 const ms_Decays[eNumDelays];
};

class CAudioFilterPitchShift : public CAudioFilter
{
public:
    CAudioFilterPitchShift();

    float32 GetPitchFactor() const;

    //Get maximum latency in seconds
    float32 GetLatency() const;

    void SetPitchFactor(float32 PitchFactor);

    //Set maximum latency in seconds
    void SetLatency(float32 LatencyTime);

    float32 Filter(float32 In);

protected:
    virtual void OnSampleRateChanged();

private:
    int32 m_Next;
    std::vector<float32> m_Buf;
    float32 m_PitchFactor;
    float32 m_Latency; //Latency in seconds
    float32 m_SweepMax;
    float32 m_SweepSpeed;
    float32 m_SweepPos;

    void UpdateSweepMax();
    void UpdateBufferSize();
};

class CAudioFilterRingModulator : public CAudioFilter
{
public:
    CAudioFilterRingModulator();

    float32 GetFrequency() const;

    void SetFrequency(float32 Frequency);

    float32 Filter(float32 In);

protected:
    virtual void OnSampleRateChanged();

private:
    CAudioWaveSine m_Wave;
};

class CAudioFilterDistortion : public CAudioFilter
{
public:
    CAudioFilterDistortion();

    float32 GetDistortion() const;
    void SetDistortion(float32 Distortion);

    float32 Filter(float32 In);

private:
    float32 m_Distortion;
    float32 m_Exponent;
    float32 m_ScaleFactor;
};

//Base class of low-pass or high-pass filter with a cutoff frequency
class CAudioFilterPass : public CAudioFilter
{
public:
    CAudioFilterPass();

    float32 GetCutoffFrequency() const;
    void SetCutoffFrequency(float32 CutoffFreq);

protected:
    float32 m_CutoffFreq;
    CDSPFilterIIR m_IIRLPF;

    virtual void OnSampleRateChanged();
};

class CAudioFilterLowPass : public CAudioFilterPass
{
public:
    float32 Filter(float32 In);
};

class CAudioFilterHighPass : public CAudioFilterPass
{
public:
    float32 Filter(float32 In);
};

class CAudioFilterScratch : public CAudioFilter
{
public:
    CAudioFilterScratch();

    int32 GetScratchSize() const;
    void SetScratchSize(int32 Size);
    float32* GetScratchBuffer();

    int32 GetScratchSampleRate();
    void SetScratchSampleRate(float32 ScratchSampleRate);

    ///Get the volume of the input to be used while scratch sample plays
    float32 GetInputVolume() const;

    //Play the scratch sample
    void Play();

    bool IsPlaying() const;

    void Stop();

    ///Set the volume of the input to be used while scratch sample plays
    void SetInputVolume(float32 InputVolume);

    float32 Filter(float32 In);

protected:
    virtual void OnSampleRateChanged();

private:
	std::vector<float32> m_Scratch;
    float32 m_ScratchSampleRate;
    float32 m_InputVolume;
    float32 m_SampleDelta;
    float32 m_SweepPos;

    void UpdateSampleDelta();
};

class CAudioFilterPan : public CAudioFilter
{
public:
    CAudioFilterPan();

    int32 GetChannel() const;
    float32 GetPan() const;

    //Sets the channel to be 0 (left) or 1 (right)
    void SetChannel(int32 Channel);

    ///Set the pan value from 0.0 (left) through 1.0 (right), where 0.5 is middle panning
    void SetPan(float32 Pan);

    float32 Filter(float32 In);

protected:
    virtual void OnMagnitudeChanged();

private:
    float32 m_Pan;
    int32 m_Channel;
    float32 m_Factor;

    void UpdateFactor();
};

}	// namespace Audio
}	// namespace WT

