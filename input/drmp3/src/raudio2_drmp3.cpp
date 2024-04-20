#include "raudio2_drmp3.h"
#include <array>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"

using namespace std::literals;

#ifndef RAUDIO2_STANDALONE_PLUGIN
#include "raudio2/raudio2.h"

#define DRMP3_MALLOC RAUDIO2_MALLOC
#define DRMP3_REALLOC RAUDIO2_REALLOC
#define DRMP3_FREE RAUDIO2_FREE
#endif

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include <dr_mp3.h> // MP3 loading functions

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return DRMP3_MakeInputPlugin(plugin);
}
#endif

bool DRMP3_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = DRMP3_Open;
    plugin->seek = DRMP3_Seek;
    plugin->read = DRMP3_Read;
    plugin->close = DRMP3_Close;
    plugin->getValue = DRMP3_GetValue;

    return true;
}

static size_t DRMP3_OnRead(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)pUserData;

    if (!file)
        return 0;

    return file->read(file->handle, pBufferOut, bytesToRead);
}

static drmp3_bool32 DRMP3_OnSeek(void* pUserData, int offset, drmp3_seek_origin origin)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)pUserData;

    if (!file)
        return DRMP3_FALSE;

    return file->seek(file->handle, offset, (origin == drmp3_seek_origin_current) ? RAUDIO2_SEEK_CUR : RAUDIO2_SEEK_SET) == 0;
}

bool DRMP3_Open(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;

    drmp3* ctxMp3 = (drmp3*)calloc(1, sizeof(drmp3));
    bool success = drmp3_init(ctxMp3, DRMP3_OnRead, DRMP3_OnSeek, wave->file, nullptr) == DRMP3_TRUE;

    wave->ctxData = ctxMp3;

    if (success && ctxMp3 != nullptr)
    {
        wave->sampleFormat = RAUDIO2_SAMPLE_FORMAT_F32;
        wave->sampleRate = (int32_t)ctxMp3->sampleRate;
        wave->channels = (int32_t)ctxMp3->channels;
        wave->frameCount = (int64_t)drmp3_get_pcm_frame_count(ctxMp3);
        return true;
    }
    return false;
}

bool DRMP3_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;
    if (!wave->ctxData)
        return false;

    if (positionInFrames <= 0)
        return drmp3_seek_to_start_of_stream((drmp3*)wave->ctxData) == DRMP3_TRUE;

    return drmp3_seek_to_pcm_frame((drmp3*)wave->ctxData, (drmp3_uint64)positionInFrames) == DRMP3_TRUE;
}

int64_t DRMP3_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    return drmp3_read_pcm_frames_f32((drmp3*)wave->ctxData, (drmp3_uint64)framesToRead, (float*)bufferOut);
}

bool DRMP3_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    drmp3_uninit((drmp3*)wave->ctxData);
    DRMP3_FREE(wave->ctxData);
    wave->ctxData = nullptr;
    return true;
}

bool DRMP3_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("dr_mp3"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 4> extensions{ ".mp3", ".mp2", ".mp1", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto ctxMp3 = (drmp3*)wave->ctxData;
        if (!ctxMp3)
            break;

        switch (keyHash)
        {
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
