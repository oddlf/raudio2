#pragma once

#include "AudioStream.h"
#include <memory>
#include "raudio2/raudio2_archiveplugin.hpp"
#include "raudio2/raudio2_inputplugin.hpp"
#include "VirtualIO.h"

struct AudioData;
class AudioDevice;

// Music, audio stream, anything longer than ~10 seconds should be streamed
class Music
{
private:
    int32_t ID{};
    std::shared_ptr<AudioStream> stream; // Audio stream
    int64_t frameCount{};                // Total number of frames (considering channels)
    bool looping{};                      // Music looping enable

    VirtualIOWrapper file;
    VirtualIOWrapper archiveFile;

    RAudio2_Archive archive;
    void* archiveFileCtx{};
    RAudio2_WaveInfo waveInfo;
    ra::ArchivePlugin archivePlugin;
    ra::InputPlugin inputPlugin;

    static int32_t Load(AudioDevice& audioDevice, const char* fileName, bool streamFile, VirtualIOWrapper&& file);

public:
    Music();

    Music(Music const&) = delete;
    Music& operator=(Music const&) = delete;

    static int32_t Load(AudioDevice& audioDevice, const char* fileName, bool streamFile);
    static int32_t LoadFromMemory(AudioDevice& audioDevice, const char* fileType, const unsigned char* dataIn, int64_t dataSize);

    auto GetID() const noexcept { return ID; }

    void Unload(AudioDevice& audioDevice);
    bool IsReady();
    void Play();
    bool IsPlaying();
    bool IsStopped();
    void Update(AudioData& audioData);
    void Stop();
    void Pause();
    void Resume();
    void Seek(double position);
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetPan(float pan);
    double GetTimeLength();
    double GetTimePlayed();

    bool GetValue(const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept;
    bool GetArchiveValue(const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept;
};
