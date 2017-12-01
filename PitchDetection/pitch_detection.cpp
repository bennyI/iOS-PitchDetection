//
//  pitch_detection.cpp
//  PitchCorrection
//
//  Created by zhouyu on 17/1/25.
//  Copyright © 2017年 zhouyu. All rights reserved.
//

#include "pitch_detection.hpp"



CPitchDetection::CPitchDetection(int iWindowSize, int iHopSize, int iSampleRate)
{
    m_sampleRate = iSampleRate;
    m_windowSize = iWindowSize;
    m_hopSize = iHopSize;
    m_silence = DEFAULT_PITCH_SILENCE;
    
    m_slideWindow = new CSlideWindow(m_windowSize, m_hopSize);
    m_slideWindow->SetHammWindow(false);
    m_tempDataBuf = new float[m_windowSize];
    m_totalSamples = 0;
    
    m_pitchYinfft = new_aubio_pitchyinfft(m_sampleRate, m_windowSize);

}

CPitchDetection::~CPitchDetection()
{
    if(m_slideWindow)
    {
        delete m_slideWindow;
        m_slideWindow = NULL;
    }
    if(m_tempDataBuf)
    {
        delete[] m_tempDataBuf;
        m_tempDataBuf = NULL;
    }
    if(m_pitchYinfft)
    {
        del_aubio_pitchyinfft(m_pitchYinfft);
        m_pitchYinfft = NULL;
    }
}


unsigned int CPitchDetection::PitchSetSilence(float fsilence)
{
    if(fsilence <= 0 && fsilence >= -200)
    {
        m_silence = fsilence;
        return 0;
    }
    else
    {
        printf("pitch: could not set silence to %.2f", fsilence);
        return 1;
    }
}

void CPitchDetection::Process_Yinfft(aubio_pitchyinfft_t* p, float* pInput, PitchElement* pOutput,
                                     unsigned int uiLength)
{
    fvec_t input;
    fvec_t* output = new_fvec(1);
    input.data = pInput;
    input.length = uiLength;
    aubio_pitchyinfft_do(p, &input, output);
    if(output->data[0] > 0)
    {
        pOutput->freq = m_sampleRate / (output->data[0] + 0.0);
    }
    else
    {
        pOutput->freq = 0.0;
    }
    pOutput->conf = aubio_pitchyinfft_get_confidence(m_pitchYinfft);
    
}

unsigned int CPitchDetection::SilenceDetection(float* pInput, unsigned int uiLength, float fThreshold)
{
    fvec_t input;
    input.data = pInput;
    input.length = uiLength;
    return aubio_silence_detection(&input, fThreshold);
}

void CPitchDetection::Process(const float *data, const size_t sampleSize)
{
    size_t uiSlice = 64;
    const float  * pSamples = data;

    for (size_t idx = 0; idx < sampleSize; idx += uiSlice)
    {
        if ((sampleSize - idx) < uiSlice)
        {
            uiSlice = sampleSize - idx;
        }
        m_totalSamples += uiSlice;
        float pTempData[64];
        memcpy(pTempData, pSamples, sizeof(float) * uiSlice);
        pSamples += uiSlice;
        bool ret = m_slideWindow->Process(pTempData, uiSlice);
        if (ret)
        {
            size_t centerLatency;
            size_t frontLatency;
            size_t retSize = m_slideWindow->CheckOutSlideWindow(m_tempDataBuf, centerLatency, frontLatency);
            if (retSize <= 0)
            {
                return ;
            }
            
            //do pitch detection(YinFFT)
            PitchElement curPitch;
            Process_Yinfft(m_pitchYinfft, m_tempDataBuf, &curPitch, m_windowSize);
            m_pitchVector.push_back(curPitch);

        }
        
    }


}

std::vector<PitchElement> CPitchDetection:: GetPitchData()
{
    return m_pitchVector;
}
