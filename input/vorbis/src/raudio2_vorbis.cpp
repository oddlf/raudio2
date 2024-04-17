#include "raudio2_vorbis.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"
#include <vorbis/vorbisfile.h>

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return VORBIS_MakeInputPlugin(plugin);
}
#endif

bool VORBIS_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->init = VORBIS_Init;
    plugin->open = VORBIS_Open;
    plugin->seek = VORBIS_Seek;
    plugin->read = VORBIS_Read;
    plugin->close = VORBIS_Close;
    plugin->getValue = VORBIS_GetValue;

    return true;
}

static size_t VORBIS_OnRead(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)datasource;

    if (!file)
        return 0;

    return file->read(file->handle, ptr, size * nmemb) / size;
}

static int VORBIS_OnSeek(void* datasource, ogg_int64_t offset, int whence)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)datasource;

    if (!file)
        return -1;

    return file->seek(file->handle, offset, whence);
}

static long VORBIS_OnTell(void* datasource)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)datasource;

    if (!file)
        return 0;

    return file->tell(file->handle);
}

static ov_callbacks ovCallbacks;

bool VORBIS_Init()
{
    ovCallbacks.read_func = VORBIS_OnRead;
    ovCallbacks.seek_func = VORBIS_OnSeek;
    ovCallbacks.close_func = nullptr;
    ovCallbacks.tell_func = VORBIS_OnTell;
    return true;
}

bool VORBIS_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    auto vorbisFile = std::make_unique<OggVorbis_File>();
    if (!vorbisFile)
        return false;

    if (ov_open_callbacks(file.getVirtualIO(), vorbisFile.get(), nullptr, 0, ovCallbacks) == 0)
    {
        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);

        auto info = ov_info(vorbisFile.get(), -1);

        wave.setSampleRate((int32_t)info->rate);
        wave.setChannels((int32_t)info->channels);

        wave.setFrameCount(ov_pcm_total(vorbisFile.get(), -1));

        wave.setCtxData(vorbisFile.release());

        return true;
    }
    return false;
}

bool VORBIS_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;
    if (!wave->ctxData)
        return false;

    auto vorbisFile = (OggVorbis_File*)wave->ctxData;

    if (positionInFrames <= 0)
        positionInFrames = 0;

    return ov_pcm_seek(vorbisFile, positionInFrames) == 0;
}

int64_t VORBIS_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto vorbisFile = (OggVorbis_File*)wave->ctxData;

    auto numBytes = (int)(framesToRead * wave->channels * 2);
    int currentSection{};
    int totalBytesRead{};

    while (numBytes >= wave->channels * 2)
    {
        auto bytesRead = ov_read(vorbisFile, (char*)bufferOut, numBytes, 0, 2, 1, &currentSection);

        if (bytesRead == OV_HOLE)
        {
            continue;
        }
        else if (bytesRead <= 0)
        {
            break;
        }
        else
        {
            totalBytesRead += bytesRead;
            bufferOut = (void*)((unsigned char*)bufferOut + bytesRead);
            numBytes -= bytesRead;
        }
    }

    return (uint64_t)(totalBytesRead / (wave->channels * 2));
}

bool VORBIS_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    auto vorbisFile = (OggVorbis_File*)wave->ctxData;

    ov_clear(vorbisFile);
    wave->ctxData = nullptr;
    return true;
}

static bool VORBIS_charequalsi(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

static bool VORBIS_strcmpi(const std::string_view a, const std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin(), VORBIS_charequalsi);
}

static bool VORBIS_getMetadata(const std::string_view comment, const std::string_view tagName, RAudio2_Value& valueOut)
{
    if (!VORBIS_strcmpi(tagName, comment))
        return false;

    return ra::MakeValue(comment.substr(tagName.size()), valueOut);
}

static bool VORBIS_getMetadata(const vorbis_comment* tags, const std::string_view tagName, RAudio2_Value& valueOut)
{
    for (auto i = 0; i < tags->comments; i++)
    {
        if (VORBIS_getMetadata(std::string_view(tags->user_comments[i], tags->comment_lengths[i]), tagName, valueOut))
            return true;
    }
    valueOut = {};
    return false;
}

bool VORBIS_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("vorbis"sv, *valueOut);
        return true;
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 7> extensions{ ".ogg", ".oga", ".ogm", ".ogv", ".ogx", ".spx", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto vorbisFile = (OggVorbis_File*)wave->ctxData;
        if (!vorbisFile)
            break;

        auto tags = ov_comment(vorbisFile, -1);

        constexpr auto vobTagArtist = "artist="sv;
        constexpr auto vobTagAlbum = "album="sv;
        constexpr auto vobTagComment = "comment="sv;
        constexpr auto vobTagDate = "date="sv;
        constexpr auto vobTagGenre = "genre="sv;
        constexpr auto vobTagTitle = "title="sv;
        constexpr auto vobTagTrackNumber = "tracknumber="sv;

        constexpr auto tagArtist = vobTagArtist.substr(0, vobTagArtist.size() - 1);
        constexpr auto tagAlbum = vobTagAlbum.substr(0, vobTagAlbum.size() - 1);
        constexpr auto tagComment = vobTagComment.substr(0, vobTagComment.size() - 1);
        constexpr auto tagDate = vobTagDate.substr(0, vobTagDate.size() - 1);
        constexpr auto tagGenre = vobTagGenre.substr(0, vobTagGenre.size() - 1);
        constexpr auto tagTitle = vobTagTitle.substr(0, vobTagTitle.size() - 1);
        constexpr auto tagTrackNumber = vobTagTrackNumber.substr(0, vobTagTrackNumber.size() - 1);

        switch (keyHash)
        {
        case ra::str2int(tagArtist): {
            return VORBIS_getMetadata(tags, vobTagArtist, *valueOut);
        }
        case ra::str2int(tagAlbum): {
            return VORBIS_getMetadata(tags, vobTagAlbum, *valueOut);
        }
        case ra::str2int(tagComment): {
            return VORBIS_getMetadata(tags, vobTagComment, *valueOut);
        }
        case ra::str2int(tagDate): {
            return VORBIS_getMetadata(tags, vobTagDate, *valueOut);
        }
        case ra::str2int(tagGenre): {
            return VORBIS_getMetadata(tags, vobTagGenre, *valueOut);
        }
        case ra::str2int(tagTitle): {
            return VORBIS_getMetadata(tags, vobTagTitle, *valueOut);
        }
        case ra::str2int(tagTrackNumber): {
            return VORBIS_getMetadata(tags, vobTagTrackNumber, *valueOut);
        }
        case ra::str2int("bps"): {
            auto bitrate = ov_bitrate(vorbisFile, -1);
            if (bitrate >= 0)
                return ra::MakeValue(bitrate, *valueOut);
            break;
        }
        case ra::str2int("current_bps"): {
            auto bitrate = ov_bitrate_instant(vorbisFile);
            if (bitrate >= 0)
                return ra::MakeValue(bitrate, *valueOut);
            break;
        }
        case ra::str2int("format"): {
            return ra::MakeValue("VORBIS"sv, *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
