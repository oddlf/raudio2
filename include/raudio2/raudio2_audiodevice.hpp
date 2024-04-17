#pragma once

#include "raudio2/raudio2.h"
#include "raudio2/raudio2_audiostream.hpp"
#include "raudio2/raudio2_music.hpp"
#include <utility>

namespace ra
{
    // AudioDevice (calls Close() on destruction)
    class AudioDevice
    {
    protected:
        RAUDIO2_HANDLE raHandle{};

        AudioDevice(RAUDIO2_HANDLE handle) noexcept : raHandle(handle){};

    public:
        AudioDevice() noexcept = default;
        virtual ~AudioDevice() noexcept { Close(); }

        AudioDevice(AudioDevice const&) noexcept = delete;
        AudioDevice& operator=(AudioDevice const&) noexcept = delete;

        AudioDevice(AudioDevice&& other) noexcept
        {
            raHandle = other.raHandle;
            other.raHandle = nullptr;
        }
        AudioDevice& operator=(AudioDevice&& other) noexcept
        {
            if (this != &other)
            {
                raHandle = other.raHandle;
                other.raHandle = nullptr;
            }
            return *this;
        }

        auto Handle() const noexcept { return raHandle; }

        explicit operator bool() const noexcept { return IsReady(); }

        bool Init(int32_t flags = RAUDIO2_FLAG_AUTOUPDATE) noexcept
        {
            if (raHandle)
                return false;

            raHandle = RAudio2_InitAudioDevice(flags);
            return raHandle != nullptr && IsReady();
        }
        bool Init(RAudio2_SampleFormat format, int32_t sampleRate, int32_t channels, int32_t flags = RAUDIO2_FLAG_AUTOUPDATE) noexcept
        {
            if (raHandle)
                return false;

            raHandle = RAudio2_InitAudioDevice2((int32_t)format, sampleRate, channels, flags);
            return raHandle != nullptr && IsReady();
        }
        bool Close() noexcept
        {
            if (!raHandle)
                return false;

            RAudio2_CloseAudioDevice(raHandle);
            raHandle = {};
            return true;
        }
        bool IsReady() const noexcept { return RAudio2_IsAudioDeviceReady(raHandle); }
        auto RegisterPlugin(const char* filePath, RAudio2_PluginType pluginType, bool append) const noexcept
        {
            return RAudio2_RegisterPlugin(raHandle, filePath, pluginType, append);
        }
        auto GetFormat() const noexcept { return RAudio2_GetAudioDeviceFormat(raHandle); }
        auto GetChannels() const noexcept { return RAudio2_GetAudioDeviceChannels(raHandle); }
        auto GetSampleRate() const noexcept { return RAudio2_GetAudioDeviceSampleRate(raHandle); }
        void SetMasterVolume(float volume) const noexcept { RAudio2_SetMasterVolume(raHandle, volume); }

        void SetAudioStreamDefaultBufferSize(int32_t size) { RAudio2_SetAudioStreamDefaultBufferSize(raHandle, size); }

        void AttachAudioMixedProcessor(AudioCallback processor) const noexcept { RAudio2_AttachAudioMixedProcessor(raHandle, processor); }
        void DetachAudioMixedProcessor(AudioCallback processor) const noexcept { RAudio2_DetachAudioMixedProcessor(raHandle, processor); }

        std::pair<AudioStream, bool> LoadAudioStream(RAudio2_SampleFormat sampleFormat, int32_t sampleRate, int32_t channels) noexcept
        {
            auto pair = std::make_pair(AudioStream(), true);
            pair.second = pair.first.Load(raHandle, sampleFormat, sampleRate, channels);
            return pair;
        }
        std::pair<Music, bool> LoadMusic(const char* fileName, bool streamFile) noexcept
        {
            auto pair = std::make_pair(Music(), true);
            pair.second = pair.first.Load(raHandle, fileName, streamFile);
            return pair;
        }
        std::pair<Music, bool> LoadMusic(const char* fileType, const unsigned char* data, int64_t dataSize) noexcept
        {
            auto pair = std::make_pair(Music(), true);
            pair.second = pair.first.Load(raHandle, fileType, data, dataSize);
            return pair;
        }

        bool getValue(const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept
        {
            return RAudio2_GetAudioDeviceValue(raHandle, key, keyLength, valueOut);
        }

        bool getValue(const std::string_view key, RAudio2_Value* valueOut) const noexcept
        {
            return RAudio2_GetAudioDeviceValue(raHandle, key.data(), (int32_t)key.size(), valueOut);
        }

        bool getBool(const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getBool(key);
            }
            return {};
        }

        int64_t getInt64(const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getInt64(key);
            }
            return {};
        }

        double getDouble(const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getDouble(key);
            }
            return {};
        }

        const char* getStringChar(const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringChar(key);
            }
            return {};
        }

        const std::string_view getStringView(const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringView(key);
            }
            return {};
        }

        const char** getStringCharArray(const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringCharArray(key);
            }
            return {};
        }

        const char** getArchivePluginNames() const noexcept { return getStringCharArray("archive_plugins"sv); }

        const char** getInputPluginNames() const noexcept { return getStringCharArray("input_plugins"sv); }
    };

    // Non owning AudioDevice (doesn't call Close() on destruction)
    class AudioDeviceView : public AudioDevice
    {
    public:
        AudioDeviceView(AudioDevice& audioDevice) noexcept : AudioDeviceView(audioDevice.Handle()){};
        AudioDeviceView(RAUDIO2_HANDLE handle) noexcept : AudioDevice(handle){};
        ~AudioDeviceView() noexcept { raHandle = {}; }

        AudioDeviceView(AudioDeviceView const&) noexcept = default;
        AudioDeviceView& operator=(AudioDeviceView const&) noexcept = default;

        AudioDeviceView(AudioDeviceView&& other) noexcept = default;
        AudioDeviceView& operator=(AudioDeviceView&& other) noexcept = default;
    };
}
