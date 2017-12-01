#include <string.h>
#include "fft_wrapper.hpp"
#include "mayer_fft.h"

#ifndef PI
#define PI (float)3.14159265358979323846
#endif

#define arlog2(x) (log(x) * 1.44269504088896340736)

CFFTWrapper::CFFTWrapper(size_t nfft)//, bool isForward, size_t step=0)
{
    m_nFft = nfft;
    m_tmpFftData = new float[nfft];
    m_halfSize = nfft / 2;
    m_LOG_N = round(arlog2(nfft));

#ifdef __ACCFORIPHONE__

    fftsetup = vDSP_create_fftsetup(m_LOG_N, kFFTRadix2);

    tempSplitComplex.realp = new float[m_halfSize + 1];
    memset(tempSplitComplex.realp, 0, sizeof(float) * (m_halfSize + 1));
    tempSplitComplex.imagp = new float[m_halfSize + 1];
    memset(tempSplitComplex.imagp, 0, sizeof(float) * (m_halfSize + 1));

#endif

    m_fftfreqre = new float[m_halfSize + 1];
    m_fftfreqim = new float[m_halfSize + 1];
    memset(m_fftfreqre, 0, (m_halfSize + 1) * sizeof(float));
    memset(m_fftfreqim, 0, (m_halfSize + 1) * sizeof(float));

    m_halfmag = new float[m_halfSize + 1];
    m_halfmagpower = new float[m_halfSize + 1];
    memset(m_halfmag, 0, (m_halfSize + 1) * sizeof(float));
    memset(m_halfmagpower, 0, (m_halfSize + 1) * sizeof(float));
}

CFFTWrapper::~CFFTWrapper()
{
    if (m_tmpFftData)
    {
        delete [] m_tmpFftData;
        m_tmpFftData = NULL;
    }
    if (m_fftfreqre) {
        delete[] m_fftfreqre;
        m_fftfreqre = NULL;
    }
    if (m_fftfreqim) {
        delete[] m_fftfreqim;
        m_fftfreqim = NULL;
    }
    if (m_halfmag) {
        delete[] m_halfmag;
        m_halfmag = NULL;
    }
    if (m_halfmagpower) {
        delete[] m_halfmagpower;
        m_halfmagpower = NULL;
    }

#ifdef __ACCFORIPHONE__
    if (fftsetup) {
        vDSP_destroy_fftsetup(fftsetup);
        fftsetup = NULL;
    }
    if (tempSplitComplex.realp) {
        delete[] tempSplitComplex.realp;
        tempSplitComplex.realp = NULL;
    }
    if (tempSplitComplex.imagp) {
        delete[] tempSplitComplex.imagp;
        tempSplitComplex.imagp = NULL;
    }
#endif
}


void CFFTWrapper::FftForward(const float input[], float outputRe[], float outputIm[])
{
#ifdef __ACCFORIPHONE__
    vDSP_ctoz((DSPComplex*)input, 2, &tempSplitComplex, 1, m_nFft / 2);
    vDSP_fft_zrip(fftsetup, &tempSplitComplex, 1, m_LOG_N, kFFTDirection_Forward);
    for (int i = 0; i < m_nFft / 2; i++) {
        outputRe[i] = tempSplitComplex.realp[i];
        outputIm[i] = tempSplitComplex.imagp[i];
    }
    float scale = 0.5;
    vDSP_vsmul(outputRe, 1, &scale, outputRe, 1, m_nFft / 2); //vdsp的结果比mayerfft结果大两倍，scale后保证二者的输出结果一致
    vDSP_vsmul(outputIm, 1, &scale, outputIm, 1, m_nFft / 2);
    return;
#else
    size_t ti;
    size_t nfft;
    size_t hnfft;

    nfft = m_nFft;
    hnfft = nfft / 2;

    memcpy(m_tmpFftData, input, sizeof(float) * nfft);

    MayerFft::mayer_realfft((int)nfft, m_tmpFftData);

    outputIm[0] = 0;
    memcpy(outputRe, m_tmpFftData, sizeof(float) * hnfft);
    for (ti = 0; ti < hnfft; ti++) {
        outputIm[ti + 1] = m_tmpFftData[nfft - 1 - ti];
    }

    outputRe[hnfft] = m_tmpFftData[hnfft];
    outputIm[hnfft] = 0;

#endif
}

void CFFTWrapper::FftInverse(const float inputRe[], const float inputIm[], float output[])
{
#ifdef __ACCFORIPHONE__
    DSPSplitComplex tsc;

    tsc.realp = (float*)inputRe;
    tsc.imagp = (float*)inputIm;


    vDSP_fft_zrip(fftsetup, &tsc, 1, m_LOG_N, kFFTDirection_Inverse);
    vDSP_ztoc(&tsc, 1, (DSPComplex*)output, 2, m_nFft / 2);

//    float scale = 0.5/m_nFft;
//    vDSP_vsmul(output, 1, &scale, output, 1, m_nFft); //如果需要真实的原始数据，则需要对结果乘以1/2N的系数
    return;
#else
    size_t ti;
    size_t nfft;
    size_t hnfft;

    nfft = m_nFft;
    hnfft = nfft / 2;

    memcpy(m_tmpFftData, inputRe, sizeof(float) * hnfft);
    for (ti = 0; ti < hnfft; ti++) {
        m_tmpFftData[nfft - 1 - ti] = inputIm[ti + 1];
    }
    m_tmpFftData[hnfft] = inputRe[hnfft];

    MayerFft::mayer_realifft((int)nfft, m_tmpFftData);

    memcpy(output, m_tmpFftData, sizeof(float) * nfft);

    //float scale = 0.5/m_nFft;
//    for (int i=0; i<m_nFft; i++) {
//        output[i]*=scale;
//    }
#endif
}

void CFFTWrapper::TransCompspec(const float inputRe[], const float inputIm[], float comSpec[])
{
    comSpec[0] = inputRe[0];
    comSpec[m_nFft / 2] = inputIm[0];
    for(int i = 0; i < m_nFft/2; i++){
        comSpec[i] = inputRe[i];
        comSpec[m_nFft - i] = inputIm[i];
    }
    
}

void printArray(float a[], int len, bool continues, const char* title) {
    printf("================%s==============\n", title);
    for (int i = 0; i < len; i++) {
        printf("%f ", a[i]);
        if (!continues)printf("\n");
    }
}
void CFFTWrapper::CalcCepstrum(const float inPut[], float outPut[]) {

#ifdef __ACCFORIPHONE__

    vDSP_ctoz((DSPComplex*)inPut, 2, &tempSplitComplex, 1, m_halfSize);
    vDSP_fft_zrip(fftsetup, &tempSplitComplex, 1, m_LOG_N, kFFTDirection_Forward);
    //remove DC
    tempSplitComplex.realp[0] = 0;

    float zxk = tempSplitComplex.imagp[0];
    vDSP_zvabs(&tempSplitComplex, 1, m_halfmag, 1, m_halfSize);
    vDSP_vsq(m_halfmag, 1, m_halfmagpower, 1, m_halfSize);

    memcpy(tempSplitComplex.realp, m_halfmagpower, m_halfSize * sizeof(float));
    memset(tempSplitComplex.imagp, 0, m_halfSize * sizeof(float));

    tempSplitComplex.imagp[0] = zxk * zxk;
    vDSP_fft_zrip(fftsetup, &tempSplitComplex, 1, m_LOG_N, kFFTDirection_Inverse);
    vDSP_ztoc(&tempSplitComplex, 1, (DSPComplex*)outPut, 2, m_halfSize);

    // Normalize
    float tf = (float)1 / outPut[0];
    vDSP_vsmul(outPut, 1, &tf, outPut, 1, m_nFft);
    outPut[0] = 1;

#else

    //1 进行fft变换
    this->FftForward(inPut, m_fftfreqre, m_fftfreqim);
    //2 去除直流分量，保留下第一项中的虚部
    m_fftfreqre[0] = 0;
    float zxk = m_fftfreqim[0];
    //3 求频域下幅值的平方，第一项为原来第一项虚部的平方
    for (size_t i = 0; i < m_halfSize; i++)
    {
        m_fftfreqre[i] = m_fftfreqre[i] * m_fftfreqre[i] + m_fftfreqim[i] * m_fftfreqim[i];
        m_fftfreqim[i] = 0;
    }
    m_fftfreqre[0] = zxk * zxk;
    //4 对幅值平方进行反向fft计算
    this->FftInverse(m_fftfreqre, m_fftfreqim, outPut);
    //5 获取归一化系数
    float tf = (float)1 / outPut[0];
    //6 进行归一化
    for (size_t i = 0; i < m_nFft; i++)
    {
        outPut[i] = outPut[i] * tf;
    }
    outPut[0] = 1;

#endif
}
