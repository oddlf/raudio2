#include "raudio2_wav.h"
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"

#ifndef RAUDIO2_STANDALONE_PLUGIN
#include "raudio2/raudio2.h"

#define DRWAV_MALLOC RAUDIO2_MALLOC
#define DRWAV_REALLOC RAUDIO2_REALLOC
#define DRWAV_FREE RAUDIO2_FREE
#endif

#include <array>

#define DR_WAV_IMPLEMENTATION
#define DR_WAV_NO_STDIO
#define DR_WAV_NO_WCHAR
#include <dr_wav.h> // WAV loading functions

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return WAV_MakeInputPlugin(plugin);
}
#endif

bool WAV_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = WAV_Open;
    plugin->seek = WAV_Seek;
    plugin->read = WAV_Read;
    plugin->close = WAV_Close;
    plugin->getValue = WAV_GetValue;

    return true;
}

static size_t WAV_OnRead(void* pUserData, void* pBufferOut, size_t bytesToRead)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)pUserData;

    if (!file)
        return 0;

    return file->read(file->handle, pBufferOut, bytesToRead);
}

static drwav_bool32 WAV_OnSeek(void* pUserData, int offset, drwav_seek_origin origin)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)pUserData;

    if (!file)
        return DRWAV_FALSE;

    return file->seek(file->handle, offset, (origin == drwav_seek_origin_current) ? RAUDIO2_SEEK_CUR : RAUDIO2_SEEK_SET) >= 0;
}

bool WAV_Open(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;

    drwav* ctxWav = (drwav*)calloc(1, sizeof(drwav));
    bool success = drwav_init_with_metadata(ctxWav, WAV_OnRead, WAV_OnSeek, wave->file, 0, nullptr) == DRWAV_TRUE;

    wave->ctxData = ctxWav;

    if (success && ctxWav)
    {
        switch (ctxWav->bitsPerSample)
        {
        case 8:
        case 16: {
            wave->sampleFormat = RAUDIO2_SAMPLE_FORMAT_S16;
            break;
        }
        default: {
            wave->sampleFormat = RAUDIO2_SAMPLE_FORMAT_F32;
            break;
        }
        }

        wave->sampleRate = (int32_t)ctxWav->sampleRate;
        wave->channels = (int32_t)ctxWav->channels;
        wave->frameCount = (int64_t)ctxWav->totalPCMFrameCount;
        return true;
    }
    return false;
}

bool WAV_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
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
        return drwav_seek_to_first_pcm_frame((drwav*)wave->ctxData) == DRWAV_TRUE;

    return drwav_seek_to_pcm_frame((drwav*)wave->ctxData, (drwav_uint64)positionInFrames) == DRWAV_TRUE;
}

int64_t WAV_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    if (wave->sampleFormat == RAUDIO2_SAMPLE_FORMAT_S16)
        return drwav_read_pcm_frames_s16((drwav*)wave->ctxData, (drwav_uint64)framesToRead, (drwav_int16*)bufferOut);
    else
        return drwav_read_pcm_frames_f32((drwav*)wave->ctxData, (drwav_uint64)framesToRead, (float*)bufferOut);

    return 0;
}

bool WAV_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    drwav_uninit((drwav*)wave->ctxData);
    free(wave->ctxData);
    return true;
}

static bool WAV_getMetadata(drwav_metadata* metadata, drwav_metadata_type type, RAudio2_Value& valueOut)
{
    for (; metadata; metadata++)
    {
        switch (metadata->type & type)
        {
        case drwav_metadata_type_list_info_album: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_artist: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_comment: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_copyright: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_date: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_genre: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_software: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_title: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        case drwav_metadata_type_list_info_tracknumber: {
            auto str = std::string_view(metadata->data.infoText.pString, metadata->data.infoText.stringLength);
            return ra::MakeValue(str, valueOut);
        }
        default:
            break;
        }
    }
    valueOut = {};
    return false;
}

bool WAV_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("dr_wav"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 5> extensions{ ".wav", ".wave", ".aif", ".aiff", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto ctxWav = (drwav*)wave->ctxData;
        if (!ctxWav)
            break;

        switch (keyHash)
        {
        case ra::str2int("album"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_album, *valueOut);
        }
        case ra::str2int("artist"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_artist, *valueOut);
        }
        case ra::str2int("comment"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_comment, *valueOut);
        }
        case ra::str2int("copyright "): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_copyright, *valueOut);
        }
        case ra::str2int("date"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_date, *valueOut);
        }
        case ra::str2int("genre"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_genre, *valueOut);
        }
        case ra::str2int("software"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_software, *valueOut);
        }
        case ra::str2int("title"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_title, *valueOut);
        }
        case ra::str2int("tracknumber"): {
            return WAV_getMetadata(ctxWav->pMetadata, drwav_metadata_type_list_info_tracknumber, *valueOut);
        }
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            auto val = (int64_t)ctxWav->fmt.avgBytesPerSec * 8;
            return ra::MakeValue(val, *valueOut);
        }
        case ra::str2int("format"): {
            return ra::MakeValue("WAVE"sv, *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
