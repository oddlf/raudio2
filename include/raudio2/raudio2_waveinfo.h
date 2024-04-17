#ifndef RAUDIO2_WAVEINFO_H
#define RAUDIO2_WAVEINFO_H

#include "raudio2_common.h"
#include "raudio2_virtualio.h"

typedef struct RAudio2_WaveInfo {
    int32_t sampleFormat;    // Sample format (RAudio2_SampleFormat, signed 16 bit, float 32 bit, ...)
    int32_t sampleRate;      // Frequency (samples per second)
    int32_t channels;        // Number of channels (1-mono, 2-stereo, ...)
    int64_t frameCount;      // Total number of frames (considering channels)
    RAudio2_VirtualIO* file; // Audio file
    void* ctxData;           // Audio context data, depends on type

#ifdef __cplusplus
    RAudio2_WaveInfo() : sampleFormat{}, sampleRate{}, channels{}, frameCount{}, file{}, ctxData{}
    {
    }
#endif
} RAudio2_WaveInfo;

#endif // RAUDIO2_WAVEINFO_H
