#pragma once

#include "raudio2_waveinfo.h"
#include "raudio2_virtualio.hpp"

namespace ra
{
    class WaveInfo
    {
    private:
        RAudio2_WaveInfo* wave;

    public:
        WaveInfo(RAudio2_WaveInfo* wave_) noexcept : wave(wave_) {}
        virtual ~WaveInfo() = default;

        explicit operator bool() const noexcept
        {
            return wave != nullptr &&
                   wave->file != nullptr &&
                   wave->file->handle != nullptr &&
                   wave->ctxData != nullptr;
        }

        bool hasValidWave() const noexcept
        {
            return wave != nullptr;
        }

        bool hasValidFile() const noexcept
        {
            return wave != nullptr &&
                   wave->file != nullptr &&
                   wave->file->handle != nullptr;
        }

        bool hasValidCtxData() const noexcept
        {
            return wave != nullptr &&
                   wave->ctxData != nullptr;
        }

        auto getSampleFormat() const noexcept { return wave->sampleFormat; }
        auto getSampleRate() const noexcept { return wave->sampleRate; }
        auto getChannels() const noexcept { return wave->channels; }
        auto getFrameCount() const noexcept { return wave->frameCount; }
        VirtualIO getFile() const noexcept { return wave->file; }
        auto getCtxData() const noexcept { return wave->ctxData; }

        void setSampleFormat(RAudio2_SampleFormat sampleFormat) noexcept { wave->sampleFormat = (int32_t)sampleFormat; }
        void setSampleRate(int32_t sampleRate) noexcept { wave->sampleRate = sampleRate; }
        void setChannels(int32_t channels) noexcept { wave->channels = channels; }
        void setFrameCount(int64_t frameCount) noexcept { wave->frameCount = frameCount; }
        void setFile(VirtualIO file) noexcept { wave->file = file.getVirtualIO(); }
        void setCtxData(void* ctxData) noexcept { wave->ctxData = ctxData; }

        int64_t calculateBps() const noexcept
        {
            switch (wave->sampleFormat)
            {
            case RAUDIO2_SAMPLE_FORMAT_U8:
                return (int64_t)wave->sampleRate * (int64_t)wave->channels * 8;
            case RAUDIO2_SAMPLE_FORMAT_S16:
                return (int64_t)wave->sampleRate * (int64_t)wave->channels * 16;
            case RAUDIO2_SAMPLE_FORMAT_S24:
                return (int64_t)wave->sampleRate * (int64_t)wave->channels * 24;
            case RAUDIO2_SAMPLE_FORMAT_S32:
            case RAUDIO2_SAMPLE_FORMAT_F32:
                return (int64_t)wave->sampleRate * (int64_t)wave->channels * 32;
            default:
                return 0;
            }
        }

        int64_t framesToMilliseconds(int64_t positionInFrames) const noexcept
        {
            return positionInFrames / (int64_t)(wave->sampleRate * wave->channels);
        }

        double framesToSeconds(int64_t positionInFrames) const noexcept
        {
            return (double)framesToMilliseconds(positionInFrames) / 1000.0;
        }
    };
}
