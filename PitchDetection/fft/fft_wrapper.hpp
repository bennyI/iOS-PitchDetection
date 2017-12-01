//
//  fft_wrapper.hpp
//  AutoRap
//
//  Created by wangguoteng on 16/9/7.
//  Copyright © 2016年 wangguoteng. All rights reserved.
//
//    使用FFT获得频域数据
//    为了适应iOS和Android两个平台，由wrapper区分vDsp实现和软件计算实现
//
//

#ifndef FFT_WRAPPER_HPP
#define FFT_WRAPPER_HPP

#ifdef __APPLE__
#define __ACCFORIPHONE__
#endif

#ifdef __ACCFORIPHONE__
#include <Accelerate/Accelerate.h>
#endif

#include <stdlib.h>

class CFFTWrapper
{
private:
    size_t m_nFft;
    float * m_tmpFftData;

public:
    /* Constructor for FFT wrapper
     * @param nFft window size
     */
    explicit CFFTWrapper(size_t nFft);

    /* Destructor for FFT wrapper
     */
    ~CFFTWrapper();

    void FftForward(const float input[], float outputRe[], float outputIm[]);

    void FftInverse(const float inputRe[], const float inputIm[], float output[]);
    
    void TransCompspec(const float inputRe[], const float inputIm[], float comSpec[]);

    void CalcCepstrum(const float inPut[], float outPut[]);
private:

#ifdef __ACCFORIPHONE__
    DSPSplitComplex tempSplitComplex;
    OpaqueFFTSetup *fftsetup;
#endif

    int m_LOG_N;
    size_t m_halfSize;
    float *m_fftfreqre;
    float *m_fftfreqim;
    float *m_halfmag;
    float *m_halfmagpower;
};

#endif

