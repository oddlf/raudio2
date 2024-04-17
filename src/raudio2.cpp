#include "raudio2/raudio2.hpp"

#define MA_MALLOC RAUDIO2_MALLOC
#define MA_REALLOC RAUDIO2_REALLOC
#define MA_FREE RAUDIO2_FREE

#define MA_NO_JACK
#define MA_NO_FLAC
#define MA_NO_MP3
#define MA_NO_WAV

// Threading model: Default: [0] COINIT_MULTITHREADED: COM calls objects on any thread (free threading)
#define MA_COINIT_VALUE 2 // [2] COINIT_APARTMENTTHREADED: Each object has its own thread (apartment model)

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "AudioDevice.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "Music.h"
#include <span>
#include <unordered_map>
#include <vector>

RAUDIO2_HANDLE RAudio2_InitAudioDevice(int32_t flags)
{
    auto audioDevice = new AudioDevice();
    if (audioDevice)
        audioDevice->Init(flags);
    return audioDevice;
}

RAUDIO2_HANDLE RAudio2_InitAudioDevice2(int32_t format, int32_t sampleRate, int32_t channels, int32_t flags)
{
    auto audioDevice = new AudioDevice();
    if (audioDevice)
        audioDevice->Init((RAudio2_SampleFormat)format, sampleRate, channels, flags);
    return audioDevice;
}

void RAudio2_CloseAudioDevice(RAUDIO2_HANDLE handle)
{
    auto audioDevice = (AudioDevice*)handle;
    if (audioDevice)
        delete audioDevice;
}

bool RAudio2_IsAudioDeviceReady(RAUDIO2_HANDLE handle)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return audioDevice->IsReady();
}

bool RAudio2_RegisterPlugin(RAUDIO2_HANDLE handle, const char* filePath, int32_t pluginType, bool append)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return audioDevice->RegisterPlugin(filePath, (RAudio2_PluginType)pluginType, append);
}

int32_t RAudio2_GetAudioDeviceFormat(RAUDIO2_HANDLE handle)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return (int32_t)audioDevice->GetFormat();
}

int32_t RAudio2_GetAudioDeviceSampleRate(RAUDIO2_HANDLE handle)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return audioDevice->GetSampleRate();
}

int32_t RAudio2_GetAudioDeviceChannels(RAUDIO2_HANDLE handle)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return audioDevice->GetChannels();
}

void RAudio2_SetMasterVolume(RAUDIO2_HANDLE handle, float volume)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    audioDevice->SetMasterVolume(volume);
}

float RAudio2_GetMasterVolume(RAUDIO2_HANDLE handle)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return audioDevice->GetMasterVolume();
}

bool RAudio2_GetAudioDeviceValue(RAUDIO2_HANDLE handle, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    if (valueOut)
        return audioDevice->GetValue(key, keyLength, valueOut);
    return {};
}

int32_t RAudio2_LoadMusic(RAUDIO2_HANDLE handle, const char* fileName, bool streamFile)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return Music::Load(*audioDevice, fileName, streamFile);
}

int32_t RAudio2_LoadMusicFromMemory(RAUDIO2_HANDLE handle, const char* fileType, const unsigned char* dataIn, int64_t dataSize)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    return Music::LoadFromMemory(*audioDevice, fileType, dataIn, dataSize);
}

bool RAudio2_IsMusicReady(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        return music->IsReady();
    return {};
}

void RAudio2_UnloadMusic(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Unload(*audioDevice);
}

void RAudio2_PlayMusic(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Play();
}

void RAudio2_PauseMusic(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Pause();
}

void RAudio2_ResumeMusic(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Resume();
}

void RAudio2_StopMusic(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Stop();
}

void RAudio2_SeekMusic(RAUDIO2_HANDLE handle, int32_t musicId, double position)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Seek(position);
}

void RAudio2_UpdateMusic(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice || audioDevice->hasAutoUpdateMusic())
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->Update(audioDevice->GetAudioData());
}

bool RAudio2_IsMusicPlaying(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        return music->IsPlaying();
    return {};
}

bool RAudio2_IsMusicStopped(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        return music->IsStopped();
    return true;
}

void RAudio2_SetMusicVolume(RAUDIO2_HANDLE handle, int32_t musicId, float volume)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->SetVolume(volume);
}

void RAudio2_SetMusicPitch(RAUDIO2_HANDLE handle, int32_t musicId, float pitch)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->SetPitch(pitch);
}

void RAudio2_SetMusicPan(RAUDIO2_HANDLE handle, int32_t musicId, float pan)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        music->SetPan(pan);
}

double RAudio2_GetMusicTimeLength(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        return music->GetTimeLength();
    return {};
}

double RAudio2_GetMusicTimePlayed(RAUDIO2_HANDLE handle, int32_t musicId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music)
        return music->GetTimePlayed();
    return {};
}

bool RAudio2_GetMusicValue(RAUDIO2_HANDLE handle, int32_t musicId, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music && valueOut)
        return music->GetValue(key, keyLength, valueOut);
    return {};
}

bool RAudio2_GetMusicArchiveValue(RAUDIO2_HANDLE handle, int32_t musicId, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto music = audioDevice->GetMusic(musicId);
    if (music && valueOut)
        return music->GetArchiveValue(key, keyLength, valueOut);
    return {};
}

int32_t RAudio2_LoadAudioStream(RAUDIO2_HANDLE handle, int32_t sampleFormat, int32_t sampleRate, int32_t channels)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto stream = AudioStream::Load(*audioDevice, (RAudio2_SampleFormat)sampleFormat, sampleRate, channels);
    if (stream)
        return stream->GetID();
    return {};
}

bool RAudio2_IsAudioStreamReady(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        return stream->IsReady();
    return {};
}

void RAudio2_UnloadAudioStream(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->Unload(*audioDevice);
}

void RAudio2_UpdateAudioStream(RAUDIO2_HANDLE handle, int32_t streamId, const void* dataIn, int64_t frameCount)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->Update(dataIn, frameCount);
}

bool RAudio2_IsAudioStreamProcessed(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        return stream->IsProcessed();
    return {};
}

void RAudio2_PlayAudioStream(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->Play();
}

void RAudio2_PauseAudioStream(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->Pause();
}

void RAudio2_ResumeAudioStream(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->Resume();
}

bool RAudio2_IsAudioStreamPlaying(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        return stream->IsPlaying();
    return {};
}

bool RAudio2_IsAudioStreamStopped(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return {};

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        return stream->IsStopped();
    return true;
}

void RAudio2_StopAudioStream(RAUDIO2_HANDLE handle, int32_t streamId)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->Stop();
}

void RAudio2_SetAudioStreamVolume(RAUDIO2_HANDLE handle, int32_t streamId, float volume)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->SetVolume(volume);
}

void RAudio2_SetAudioStreamPitch(RAUDIO2_HANDLE handle, int32_t streamId, float pitch)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->SetPitch(pitch);
}

void RAudio2_SetAudioStreamPan(RAUDIO2_HANDLE handle, int32_t streamId, float pan)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->SetPan(pan);
}

void RAudio2_SetAudioStreamDefaultBufferSize(RAUDIO2_HANDLE handle, int32_t size)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    audioDevice->GetAudioData().buffer.defaultSize = size;
}

void RAudio2_SetAudioStreamCallback(RAUDIO2_HANDLE handle, int32_t streamId, AudioCallback callback)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    auto stream = audioDevice->GetAudioStream(streamId);
    if (stream)
        stream->SetCallback(callback);
}

void RAudio2_AttachAudioStreamProcessor(RAUDIO2_HANDLE handle, int32_t streamId, AudioCallback process)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    audioDevice->AttachAudioStreamProcessor(streamId, process);
}

void RAudio2_DetachAudioStreamProcessor(RAUDIO2_HANDLE handle, int32_t streamId, AudioCallback process)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    audioDevice->DetachAudioStreamProcessor(streamId, process);
}

void RAudio2_AttachAudioMixedProcessor(RAUDIO2_HANDLE handle, AudioCallback process)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    audioDevice->AttachAudioMixedProcessor(process);
}

void RAudio2_DetachAudioMixedProcessor(RAUDIO2_HANDLE handle, AudioCallback process)
{
    auto audioDevice = (AudioDevice*)handle;
    if (!audioDevice)
        return;

    audioDevice->DetachAudioMixedProcessor(process);
}
