#pragma once

#include "AudioData.h"
#include "AudioStream.h"
#include "FileIO.h"
#include <memory>
#include "Music.h"
#include "raudio2/raudio2.hpp"
#include "raudio2/raudio2_archiveplugin.hpp"
#include <thread>
#include <unordered_map>
#include <vector>

class AudioDevice
{
private:
    AudioData audioData;

    std::vector<std::unique_ptr<RAudio2_ArchivePlugin>> archivePlugins;
    std::vector<std::unique_ptr<RAudio2_InputPlugin>> inputPlugins;

    std::vector<const char*> archivePluginNames{ nullptr };
    std::vector<const char*> inputPluginNames{ nullptr };

    std::unordered_map<int32_t, std::shared_ptr<AudioStream>> streams;
    std::unordered_map<int32_t, std::unique_ptr<Music>> musics;

    std::jthread updateThread;

    void UpdateThreadFunction();

public:
    AudioDevice() = default;
    virtual ~AudioDevice();

    AudioDevice(AudioDevice const&) = delete;
    AudioDevice& operator=(AudioDevice const&) = delete;

    void Init(int32_t flags);
    void Init(RAudio2_SampleFormat sampleFormat, int32_t sampleRate, int32_t channels, int32_t flags);

    void Uninit();

    bool IsReady() const;

    bool hasAutoUpdateMusic() const noexcept { return updateThread.joinable(); }

    auto& GetAudioData() { return audioData; }
    auto& GetAudioData() const { return audioData; }

    auto& ArchivePlugins() const { return archivePlugins; }
    auto& InputPlugins() const { return inputPlugins; }

    bool RegisterPlugin(const char* filePath, RAudio2_PluginType pluginType, bool append);

    bool RegisterArchivePlugin(bool (*getArchivePlugin)(RAudio2_ArchivePlugin*), bool append);
    bool RegisterArchivePlugin(const RAudio2_ArchivePlugin& plugin, bool append);

    bool RegisterInputPlugin(bool (*getInputPlugin)(RAudio2_InputPlugin*), bool append);
    bool RegisterInputPlugin(const RAudio2_InputPlugin& plugin, bool append);

    int32_t AddAudioStream(const std::shared_ptr<AudioStream>& stream);
    int32_t AddMusic(std::unique_ptr<Music>&& music);

    AudioStream* GetAudioStream(int32_t streamId) const;
    Music* GetMusic(int32_t musicId) const;

    bool DeleteAudioStream(int32_t streamId);
    bool DeleteMusic(int32_t musicId);

    RAudio2_SampleFormat GetFormat();
    int32_t GetSampleRate();
    int32_t GetChannels();
    int32_t GetDefaultBufferSize();

    float GetMasterVolume();

    void SetMasterVolume(float volume);

    void AttachAudioStreamProcessor(int32_t streamId, AudioCallback process);

    void DetachAudioStreamProcessor(int32_t streamId, AudioCallback process);

    void AttachAudioMixedProcessor(AudioCallback process);

    void DetachAudioMixedProcessor(AudioCallback process);

    bool GetValue(const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept;
};
