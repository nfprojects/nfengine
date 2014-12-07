/**
 * @file   SoundSample.cpp
 * @author Witek902 (witek902@gmail.com)
 * @brief  Sound sample resource definition.
 */

#include "stdafx.hpp"
#include "SoundSample.hpp"
#include "../nfCommon/Logger.hpp"

namespace NFE {
namespace Resource {

struct WaveHeader
{
    char                RIFF[4];        /* RIFF Header      */ //Magic header
    unsigned long       chunkSize;      /* RIFF Chunk Size  */
    char                WAVE[4];        /* WAVE Header      */
    char                fmt[4];         /* FMT header       */
    unsigned long       subchunk1Size;  /* Size of the fmt chunk                                */
    unsigned short
    AudioFormat;    /* Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM */
    unsigned short      numOfChan;      /* Number of channels 1=Mono 2=Sterio                   */
    unsigned long       samplesPerSec;  /* Sampling Frequency in Hz                             */
    unsigned long       bytesPerSec;    /* bytes per second */
    unsigned short      blockAlign;     /* 2=16-bit mono, 4=16-bit stereo */
    unsigned short      bitsPerSample;  /* Number of bits per sample      */
    char                subchunk2ID[4]; /* "data"  string   */
    unsigned long       subchunk2Size;  /* Sampled data length    */
};


SoundInfo::SoundInfo()
{
    numChannels = 0;
    bytesPerSample = 0;
    sampleRate = 0;
    numSamples = 0;
}


SoundSample::SoundSample()
{
    mSampleData = NULL;
}

SoundSample::~SoundSample()
{
    Release();
}

void SoundSample::Release()
{
    if (mSampleData)
    {
        free(mSampleData);
        mSampleData = NULL;
    }
}

bool SoundSample::OnLoad()
{
    LOG_INFO("Loading sound resource '%s'...", mName);

    return true;
}

void SoundSample::OnUnload()
{
    Release();
}

} // namespace Resource
} // namespace NFE
