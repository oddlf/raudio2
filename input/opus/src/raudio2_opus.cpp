#include "raudio2_opus.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <opus/opusfile.h>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return OPUS_MakeInputPlugin(plugin);
}
#endif

bool OPUS_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->init = OPUS_Init;
    plugin->open = OPUS_Open;
    plugin->seek = OPUS_Seek;
    plugin->read = OPUS_Read;
    plugin->close = OPUS_Close;
    plugin->getValue = OPUS_GetValue;

    return true;
}

// Reads up to nbytes bytes of data from stream.
//    \param      stream The stream to read from.
//    \param[out] ptr    The buffer to store the data in.
//    \param      nbytes The maximum number of bytes to read.
//                       This function may return fewer, though it will not
//                       return zero unless it reaches end-of-file.
//    \return The number of bytes successfully read, or a negative value on error.
static int OPUS_OnRead(void* stream, unsigned char* ptr, int nbytes)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)stream;

    if (!file)
        return -1;

    return file->read(file->handle, ptr, nbytes);
}

// Sets the position indicator for stream.
// The new position, measured in bytes, is obtained by adding offset
// bytes to the position specified by whence. If whence is set to SEEK_SET,
// SEEK_CUR, or SEEK_END, the offset is relative to the start of the stream,
// the current position indicator, or end-of-file, respectively.
// \retval 0  Success.
// \retval -1 Seeking is not supported or an error occurred. errno need not be set.
static int OPUS_OnSeek(void* stream, opus_int64 offset, int whence)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)stream;

    if (!file)
        return -1;

    return file->seek(file->handle, offset, whence);
}

// Obtains the current value of the position indicator for stream.
// \return The current position indicator.
static opus_int64 OPUS_OnTell(void* stream)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)stream;

    if (!file)
        return 0;

    return file->tell(file->handle);
}

static OpusFileCallbacks ofCallbacks;

bool OPUS_Init()
{
    ofCallbacks.read = OPUS_OnRead;
    ofCallbacks.seek = OPUS_OnSeek;
    ofCallbacks.close = nullptr;
    ofCallbacks.tell = OPUS_OnTell;
    return true;
}

bool OPUS_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    int error{};
    auto opusFile = op_open_callbacks(file.getVirtualIO(), &ofCallbacks, nullptr, 0, &error);
    if (!opusFile)
        return false;

    if (opusFile)
    {
        wave.setCtxData(opusFile);

        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);
        wave.setSampleRate(48000);

        //auto info = op_head(opusFile, -1);
        //wave.setChannels((int32_t)info->channel_count);
        wave.setChannels(2);

        wave.setFrameCount(op_pcm_total(opusFile, -1));

        return true;
    }
    return false;
}

bool OPUS_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;
    if (!wave->ctxData)
        return false;

    auto opusFile = (OggOpusFile*)wave->ctxData;

    if (positionInFrames <= 0)
        positionInFrames = 0;

    return op_pcm_seek(opusFile, positionInFrames) == 0;
    return false;
}

int64_t OPUS_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto opusFile = (OggOpusFile*)wave->ctxData;

    auto numBytes = (int)(framesToRead * wave->channels * 2);
    int totalBytesRead{};

    while (numBytes >= wave->channels * 2)
    {
        auto samplesRead = op_read_stereo(opusFile, (opus_int16*)bufferOut, numBytes / 2);

        if (samplesRead == OP_HOLE)
        {
            continue;
        }
        else if (samplesRead <= 0)
        {
            break;
        }
        else
        {
            auto bytesRead = samplesRead * wave->channels * 2;
            totalBytesRead += bytesRead;
            bufferOut = (void*)((unsigned char*)bufferOut + bytesRead);
            numBytes -= bytesRead;
        }
    }

    return (uint64_t)(totalBytesRead / (wave->channels * 2));
}

bool OPUS_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    auto opusFile = (OggOpusFile*)wave->ctxData;

    op_free(opusFile);
    wave->ctxData = nullptr;
    return true;
}

static bool OPUS_charequalsi(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

static bool OPUS_strcmpi(const std::string_view a, const std::string_view b)
{
    return std::equal(a.begin(), a.end(), b.begin(), OPUS_charequalsi);
}

static bool OPUS_getMetadata(const std::string_view comment, const std::string_view tagName, RAudio2_Value& valueOut)
{
    if (!OPUS_strcmpi(tagName, comment))
        return false;

    return ra::MakeValue(comment.substr(tagName.size()), valueOut);
}

static bool OPUS_getMetadata(const OpusTags* tags, const std::string_view tagName, RAudio2_Value& valueOut)
{
    for (auto i = 0; i < tags->comments; i++)
    {
        if (OPUS_getMetadata(std::string_view(tags->user_comments[i], tags->comment_lengths[i]), tagName, valueOut))
            return true;
    }
    valueOut = {};
    return false;
}

bool OPUS_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("opus"sv, *valueOut);
        return true;
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 3> extensions{ ".opus", ".ogg", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto opusFile = (OggOpusFile*)wave->ctxData;
        if (!opusFile)
            break;

        auto tags = op_tags(opusFile, -1);

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
            return OPUS_getMetadata(tags, vobTagArtist, *valueOut);
        }
        case ra::str2int(tagAlbum): {
            return OPUS_getMetadata(tags, vobTagAlbum, *valueOut);
        }
        case ra::str2int(tagComment): {
            return OPUS_getMetadata(tags, vobTagComment, *valueOut);
        }
        case ra::str2int(tagDate): {
            return OPUS_getMetadata(tags, vobTagDate, *valueOut);
        }
        case ra::str2int(tagGenre): {
            return OPUS_getMetadata(tags, vobTagGenre, *valueOut);
        }
        case ra::str2int(tagTitle): {
            return OPUS_getMetadata(tags, vobTagTitle, *valueOut);
        }
        case ra::str2int(tagTrackNumber): {
            return OPUS_getMetadata(tags, vobTagTrackNumber, *valueOut);
        }
        case ra::str2int("bps"): {
            auto bitrate = op_bitrate(opusFile, -1);
            if (bitrate >= 0)
                return ra::MakeValue(bitrate, *valueOut);
            break;
        }
        case ra::str2int("current_bps"): {
            auto bitrate = op_bitrate_instant(opusFile);
            if (bitrate >= 0)
                return ra::MakeValue(bitrate, *valueOut);
            break;
        }
        case ra::str2int("format"): {
            return ra::MakeValue("OPUS"sv, *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
