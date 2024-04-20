#include "raudio2_drflac.h"
#include <array>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_virtualio.hpp"

using namespace std::literals;

#ifndef RAUDIO2_STANDALONE_PLUGIN
#include "raudio2/raudio2.h"

#define DRFLAC_MALLOC RAUDIO2_MALLOC
#define DRFLAC_REALLOC RAUDIO2_REALLOC
#define DRFLAC_FREE RAUDIO2_FREE
#endif

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#include <dr_flac.h> // FLAC loading functions

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return DRFLAC_MakeInputPlugin(plugin);
}
#endif

bool DRFLAC_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = DRFLAC_Open;
    plugin->seek = DRFLAC_Seek;
    plugin->read = DRFLAC_Read;
    plugin->close = DRFLAC_Close;
    plugin->getValue = DRFLAC_GetValue;

    return true;
}

static size_t DRFLAC_OnRead(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    auto file = ra::VirtualIO((RAudio2_VirtualIO*)pUserData);

    if (!file)
        return 0;

    return file.read(pBufferOut, bytesToRead);
}

static drflac_bool32 DRFLAC_OnSeek(void* pUserData, int offset, drflac_seek_origin origin)
{
    auto file = ra::VirtualIO((RAudio2_VirtualIO*)pUserData);

    if (!file)
        return DRFLAC_FALSE;

    return file.seek(offset, (origin == drflac_seek_origin_current) ? RAUDIO2_SEEK_CUR : RAUDIO2_SEEK_SET) == 0;
}

bool DRFLAC_Open(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;

    drflac* ctxFlac = drflac_open(DRFLAC_OnRead, DRFLAC_OnSeek, wave->file, nullptr);

    wave->ctxData = ctxFlac;

    if (wave->ctxData != nullptr)
    {
        if (ctxFlac->bitsPerSample <= 16)
            wave->sampleFormat = RAUDIO2_SAMPLE_FORMAT_S16;
        else
            wave->sampleFormat = RAUDIO2_SAMPLE_FORMAT_F32;

        wave->sampleRate = (int32_t)ctxFlac->sampleRate;
        wave->channels = (int32_t)ctxFlac->channels;
        wave->frameCount = (int64_t)ctxFlac->totalPCMFrameCount;
        return true;
    }
    return false;
}

bool DRFLAC_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
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
        return drflac__seek_to_first_frame((drflac*)wave->ctxData) == DRFLAC_TRUE;

    return drflac_seek_to_pcm_frame((drflac*)wave->ctxData, (drflac_uint64)positionInFrames) == DRFLAC_TRUE;
}

int64_t DRFLAC_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto ctxFlac = (drflac*)wave->ctxData;

    if (ctxFlac->bitsPerSample <= 16)
        return drflac_read_pcm_frames_s16(ctxFlac, (drflac_uint64)framesToRead, (drflac_int16*)bufferOut);
    else
        return drflac_read_pcm_frames_f32(ctxFlac, (drflac_uint64)framesToRead, (float*)bufferOut);
}

bool DRFLAC_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    drflac_free((drflac*)wave->ctxData, NULL);
    wave->ctxData = nullptr;
    return true;
}

bool DRFLAC_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("dr_flac"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 4> extensions{ ".flac", ".oga", ".ogg", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
        return true;
    }
    default: {
        auto ctxFlac = (drflac*)wave->ctxData;
        if (!ctxFlac)
            break;

        switch (keyHash)
        {
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            auto val = (int64_t)ctxFlac->sampleRate * (int64_t)ctxFlac->channels * (int64_t)ctxFlac->bitsPerSample;
            return ra::MakeValue(val, *valueOut);
        }
        case ra::str2int("format"): {
            return ra::MakeValue(ctxFlac->container == drflac_container_ogg ? "FLAC (OGG)" : "FLAC", *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
