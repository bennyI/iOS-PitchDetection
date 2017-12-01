//
//  pitch_detection.hpp
//  PitchCorrection
//
//  Created by zhouyu on 17/1/25.
//  Copyright © 2017年 zhouyu. All rights reserved.
//

#ifndef pitch_detection_hpp
#define pitch_detection_hpp

#include <stdio.h>
#include "slide_window.hpp"
#include "aubio.h"
#include <math.h>
#include <vector>

//音频的pitch位置数据信息
typedef struct tagPitchElement
{
    int time = 0;
    int frameIndex = 0;
    double midi = 0;
    double freq = 0;
    double conf = 0;
    int freq2Midi(double freq) {
        return int(69 + 12 * log(freq / 440) + 3);
    }
} PitchElement;

#define DEFAULT_PITCH_SILENCE -50
class CPitchDetection {

public:
    CPitchDetection(int iWindowSize, int iHopSize, int iSampleRate);
    ~CPitchDetection();
    
    void Process(const float data[], const size_t sampleSize);
    std::vector<PitchElement> GetPitchData();
    
private:
    unsigned int m_sampleRate;   //采样率
    unsigned int m_windowSize;   //buffer大小
    unsigned int m_hopSize;      //hopsize
    float m_silence;               //silence threshold
    float* m_tempDataBuf;         //当前处理数据buf

    
    CSlideWindow* m_slideWindow;
    size_t m_totalSamples;
    
    aubio_pitchyinfft_t* m_pitchYinfft;
    
    
    std::vector<PitchElement> m_pitchVector;
    
    /**
     * @brief Processs Yinfft
     * @param p aubio_pitchyinfft object
     * @param pInput input data
     * @param pOutput PitchElment
     * @param uiLength window size
     */
    void Process_Yinfft(aubio_pitchyinfft_t* p, float* pInput, PitchElement* pOutput, unsigned int uiLength);
    
    /**
     * @brief Silence Detection
     * @param pInput input data
     * @param fThreshold silence threshold
     * @param uiLength input size
     * @return 1: silence 0:not silence
     */
    unsigned int SilenceDetection(float* pInput, unsigned int uiLength, float fThreshold);
    
    /**
     * @brief: sett the silence threshold for the pitch detection
     * @param  fsilence silence level threshold under which pitch should be ignored, in dB
     * return  0: if successful, no-zero otherwise
     */
    unsigned int PitchSetSilence(float fsilence);
    
};

#endif /* pitch_detection_hpp */
