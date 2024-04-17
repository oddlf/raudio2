#pragma once

#include "AudioBuffer.h"
#include <memory>
#include "raudio2/raudio2.hpp"

class AudioDevice;

// AudioStream, custom audio stream
class AudioStream
{
private:
    int32_t ID{};

public:
    AudioBuffer* buffer{};       // Pointer to internal data used by the audio system
    AudioProcessor* processor{}; // Pointer to internal data processor, useful for audio effects

private:
    int32_t sampleSize{}; // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    int32_t sampleRate{}; // Frequency (samples per second)
    int32_t channels{};   // Number of channels (1-mono, 2-stereo, ...)

public:
    AudioStream();

    AudioStream(AudioStream const&) = delete;
    AudioStream& operator=(AudioStream const&) = delete;

    static std::shared_ptr<AudioStream> Load(AudioDevice& audioDevice, RAudio2_SampleFormat sampleFormat, int32_t sampleRate, int32_t channels);

    auto GetID() const noexcept { return ID; }
    auto GetSampleSize() const noexcept { return sampleSize; }
    auto GetSampleRate() const noexcept { return sampleRate; }
    auto GetChannels() const noexcept { return channels; }

    void Unload(AudioDevice& audioDevice);
    bool IsReady();
    void Update(const void* dataIn, int64_t samplesCount);
    bool IsProcessed();
    void Play();
    void Pause();
    void Resume();
    bool IsPlaying();
    bool IsStopped();
    void Stop();
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetPan(float pan);
    void SetCallback(AudioCallback callback);
};
