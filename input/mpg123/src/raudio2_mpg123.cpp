#include "raudio2_mpg123.h"
#include <array>
#include <mpg123.h>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_virtualio.hpp"
#include "raudio2/raudio2_waveinfo.hpp"

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return MPG123_MakeInputPlugin(plugin);
}
#endif

bool MPG123_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = MPG123_Open;
    plugin->seek = MPG123_Seek;
    plugin->read = MPG123_Read;
    plugin->close = MPG123_Close;
    plugin->getValue = MPG123_GetValue;

    return true;
}

static mpg123_ssize_t MPG123_OnRead(void* iohandle, void* buf, size_t size)
{
    auto file = ra::VirtualIO((RAudio2_VirtualIO*)iohandle);

    if (!file)
        return 0;

    return (mpg123_ssize_t)file.read(buf, (int64_t)size);
}

static off_t MPG123_OnSeek(void* iohandle, off_t offset, int whence)
{
    auto file = ra::VirtualIO((RAudio2_VirtualIO*)iohandle);

    if (!file)
        return -1;

    if (file.seek(offset, whence) != 0)
        return -1;

    return file.tell();
}

static void MPG123_Cleanup(mpg123_handle* handle)
{
    if (handle)
    {
        mpg123_close(handle);
        mpg123_delete(handle);
    }
}

bool MPG123_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    int error{};
    auto mpg123File = mpg123_new(nullptr, &error);
    if (!mpg123File)
        return false;

    mpg123_param(mpg123File, MPG123_ADD_FLAGS, MPG123_SEEKBUFFER | MPG123_FUZZY, 0);

    if (mpg123_replace_reader_handle(mpg123File, MPG123_OnRead, MPG123_OnSeek, nullptr) != MPG123_OK)
    {
        MPG123_Cleanup(mpg123File);
        return false;
    }

    constexpr std::array<int, 9> samplerates = { 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 };

    mpg123_format_none(mpg123File);
    for (auto samplerate : samplerates)
        mpg123_format(mpg123File, samplerate, MPG123_MONO | MPG123_STEREO, MPG123_ENC_FLOAT_32);

    if (mpg123_open_handle(mpg123File, file.getVirtualIO()) != MPG123_OK)
    {
        MPG123_Cleanup(mpg123File);
        return false;
    }

    long framerate{};
    int channels{};
    int mpg123Encoding{};

    if (mpg123_getformat(mpg123File, &framerate, &channels, &mpg123Encoding) != MPG123_OK)
    {
        MPG123_Cleanup(mpg123File);
        return false;
    }

    if (mpg123Encoding != MPG123_ENC_FLOAT_32)
    {
        MPG123_Cleanup(mpg123File);
        return false;
    }

    wave.setCtxData(mpg123File);

    wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_F32);
    wave.setSampleRate((int32_t)framerate);
    wave.setChannels((int32_t)channels);
    wave.setFrameCount((unsigned int)mpg123_length(mpg123File));

    return true;
}

bool MPG123_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;
    if (!wave->ctxData)
        return false;

    auto mpg123File = (mpg123_handle*)wave->ctxData;

    if (positionInFrames <= 0)
        positionInFrames = 0;

    return mpg123_seek(mpg123File, positionInFrames, 0) >= 0;
}

int64_t MPG123_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto mpg123File = (mpg123_handle*)wave->ctxData;

    size_t done = 0;
    auto bufferSize = (size_t)(framesToRead * wave->channels * 4);

    if (mpg123_read(mpg123File, bufferOut, bufferSize, &done) != MPG123_OK)
        return 0;

    return framesToRead;
}

bool MPG123_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    MPG123_Cleanup((mpg123_handle*)wave->ctxData);
    wave->ctxData = nullptr;
    return true;
}

bool MPG123_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("mpg123"sv, *valueOut);
        return true;
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 4> extensions{ ".mp3", ".mp2", ".mp1", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto mpg123File = (mpg123_handle*)wave->ctxData;
        if (!mpg123File)
            break;

        mpg123_id3v1* tag1{};
        mpg123_id3v2* tag2{};
        mpg123_id3(mpg123File, &tag1, &tag2);

        switch (keyHash)
        {
        case ra::str2int("album"): {
            if (tag2 && tag2->album)
                return ra::MakeValue(std::string_view(tag2->album->p, tag2->album->fill - 1), *valueOut);
            if (tag1)
                return ra::MakeValue(std::string_view(tag1->album), *valueOut);
            break;
        }
        case ra::str2int("artist"): {
            if (tag2 && tag2->artist)
                return ra::MakeValue(std::string_view(tag2->artist->p, tag2->artist->fill - 1), *valueOut);
            if (tag1)
                return ra::MakeValue(std::string_view(tag1->artist), *valueOut);
            break;
        }
        case ra::str2int("comment"): {
            if (tag2 && tag2->comment)
                return ra::MakeValue(std::string_view(tag2->comment->p, tag2->comment->fill - 1), *valueOut);
            if (tag1)
                return ra::MakeValue(std::string_view(tag1->comment), *valueOut);
            break;
        }
        case ra::str2int("year"): {
            if (tag2 && tag2->year)
                return ra::MakeValue(std::string_view(tag2->year->p, tag2->year->fill - 1), *valueOut);
            if (tag1)
                return ra::MakeValue(std::string_view(tag1->year), *valueOut);
            break;
        }
        case ra::str2int("title"): {
            if (tag2 && tag2->title)
                return ra::MakeValue(std::string_view(tag2->title->p, tag2->title->fill - 1), *valueOut);
            if (tag1)
                return ra::MakeValue(std::string_view(tag1->title), *valueOut);
            break;
        }
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            mpg123_frameinfo2 frameInfo;
            if (mpg123_info2(mpg123File, &frameInfo) == MPG123_OK)
            {
                auto val = (int64_t)frameInfo.bitrate * 1000;
                return ra::MakeValue(val, *valueOut);
            }
            break;
        }
        case ra::str2int("format"): {
            mpg123_frameinfo2 frameInfo;
            if (mpg123_info2(mpg123File, &frameInfo) == MPG123_OK)
            {
                switch (frameInfo.layer)
                {
                case 1:
                    return ra::MakeValue("MP1", *valueOut);
                case 2:
                    return ra::MakeValue("MP2", *valueOut);
                case 3:
                default:
                    return ra::MakeValue("MP3", *valueOut);
                }
            }
            break;
        }
        case ra::str2int("vbr"): {
            mpg123_frameinfo2 frameInfo;
            if (mpg123_info2(mpg123File, &frameInfo) == MPG123_OK)
            {
                return ra::MakeValue(frameInfo.vbr != mpg123_vbr::MPG123_CBR, *valueOut);
            }
            break;
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
