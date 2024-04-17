#pragma once

#include "raudio2/raudio2.h"
#include <string_view>

namespace ra
{
    using namespace std::literals;

    // AudioStream (calls Unload() on destruction)
    class AudioStream
    {
    protected:
        RAUDIO2_HANDLE raHandle{};
        int32_t id{};

        AudioStream(RAUDIO2_HANDLE handle, int32_t streamId) noexcept : raHandle(handle), id(streamId){};

    public:
        AudioStream() = default;
        virtual ~AudioStream() { Unload(); }

        AudioStream(AudioStream const&) noexcept = delete;
        AudioStream& operator=(AudioStream const&) noexcept = delete;

        AudioStream(AudioStream&& other) noexcept
        {
            raHandle = other.raHandle;
            id = other.id;
            other.raHandle = nullptr;
            other.id = 0;
        }
        AudioStream& operator=(AudioStream&& other) noexcept
        {
            if (this != &other)
            {
                raHandle = other.raHandle;
                id = other.id;
                other.raHandle = nullptr;
                other.id = 0;
            }
            return *this;
        }

        auto Handle() const noexcept { return raHandle; }
        auto Id() const noexcept { return id; }

        explicit operator bool() const noexcept { return IsReady(); }

        bool Load(RAUDIO2_HANDLE handle, RAudio2_SampleFormat sampleFormat, int32_t sampleRate, int32_t channels) noexcept
        {
            if (id || !handle)
                return false;

            id = RAudio2_LoadAudioStream(handle, (int32_t)sampleFormat, sampleRate, channels);
            if (id != 0)
            {
                raHandle = handle;
                return true;
            }
            raHandle = {};
            return false;
        }
        void Unload() noexcept
        {
            if (id)
            {
                RAudio2_UnloadAudioStream(raHandle, id);
                id = {};
                raHandle = {};
            }
        }
        bool IsReady() const noexcept { return RAudio2_IsAudioStreamReady(raHandle, id); }
        auto Play() const noexcept { RAudio2_PlayAudioStream(raHandle, id); }
        auto IsPlaying() const noexcept { return RAudio2_IsAudioStreamPlaying(raHandle, id); }
        auto IsStopped() const noexcept { return RAudio2_IsAudioStreamStopped(raHandle, id); }
        auto Update(const void* dataIn, int64_t samplesCount) const noexcept { RAudio2_UpdateAudioStream(raHandle, id, dataIn, samplesCount); }
        auto Stop() const noexcept { RAudio2_StopAudioStream(raHandle, id); }
        auto Pause() const noexcept { RAudio2_PauseAudioStream(raHandle, id); }
        auto Resume() const noexcept { RAudio2_ResumeAudioStream(raHandle, id); }
        auto SetVolume(float volume) const noexcept { RAudio2_SetAudioStreamVolume(raHandle, id, volume); }
        auto SetPitch(float pitch) const noexcept { RAudio2_SetAudioStreamPitch(raHandle, id, pitch); }
        auto SetPan(float pan) const noexcept { RAudio2_SetAudioStreamPan(raHandle, id, pan); }

        void SetCallback(AudioCallback callback) const noexcept { RAudio2_SetAudioStreamCallback(raHandle, id, callback); }
        void AttachProcessor(AudioCallback processor) const noexcept { RAudio2_AttachAudioStreamProcessor(raHandle, id, processor); }
        void DetachProcessor(AudioCallback processor) const noexcept { RAudio2_DetachAudioStreamProcessor(raHandle, id, processor); }
    };

    // Non owning AudioStream (doesn't call Unload() on destruction)
    class AudioStreamView : public AudioStream
    {
    public:
        AudioStreamView(AudioStream& audioStream) noexcept : AudioStreamView(audioStream.Handle(), audioStream.Id()){};
        AudioStreamView(RAUDIO2_HANDLE handle, int32_t streamId) noexcept : AudioStream(handle, streamId){};
        ~AudioStreamView() noexcept
        {
            raHandle = {};
            id = {};
        }

        AudioStreamView(AudioStreamView const&) noexcept = default;
        AudioStreamView& operator=(AudioStreamView const&) noexcept = default;

        AudioStreamView(AudioStreamView&& other) noexcept = default;
        AudioStreamView& operator=(AudioStreamView&& other) noexcept = default;
    };
}
