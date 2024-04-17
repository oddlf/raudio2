#include "raudio2_sndfile.h"
#include <array>
#include <cstring>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"
#include <sndfile.h>

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return SNDFILE_MakeInputPlugin(plugin);
}
#endif

bool SNDFILE_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->init = SNDFILE_Init;
    plugin->open = SNDFILE_Open;
    plugin->seek = SNDFILE_Seek;
    plugin->read = SNDFILE_Read;
    plugin->close = SNDFILE_Close;
    plugin->getValue = SNDFILE_GetValue;

    return true;
}

sf_count_t SNDFILE_OnGetSize(void* data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)data;

    if (!file)
        return -1;

    return file->getSize(file->handle);
}

sf_count_t SNDFILE_OnSeek(sf_count_t offset, int whence, void* data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)data;

    if (!file)
        return -1;

    return file->seek(file->handle, offset, whence);
}

sf_count_t SNDFILE_OnRead(void* ptr, sf_count_t count, void* data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)data;

    if (!file)
        return 0;

    return file->read(file->handle, ptr, count);
}

sf_count_t SNDFILE_OnTell(void* data)
{
    RAudio2_VirtualIO* file = (RAudio2_VirtualIO*)data;

    if (!file)
        return 0;

    return file->tell(file->handle);
}

static SF_VIRTUAL_IO sfVirtualIO;

bool SNDFILE_Init()
{
    sfVirtualIO.get_filelen = SNDFILE_OnGetSize;
    sfVirtualIO.seek = SNDFILE_OnSeek;
    sfVirtualIO.read = SNDFILE_OnRead;
    sfVirtualIO.write = nullptr;
    sfVirtualIO.tell = SNDFILE_OnTell;
    return true;
}

struct SNDFILE_Music {
    SNDFILE* file{ nullptr };
    SF_INFO info{};
};

bool SNDFILE_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    SF_INFO snd_info;
    memset(&snd_info, 0, sizeof(snd_info));

    auto sndfile = sf_open_virtual(&sfVirtualIO, SFM_READ, &snd_info, file.getVirtualIO());

    if (sndfile)
    {
        auto sndMusic = new SNDFILE_Music();
        if (!sndMusic)
            return false;

        sndMusic->file = sndfile;
        sndMusic->info = snd_info;

        wave.setCtxData(sndMusic);

        switch (snd_info.format & SF_FORMAT_DOUBLE)
        {
        case SF_FORMAT_PCM_24:
        case SF_FORMAT_PCM_32:
        case SF_FORMAT_FLOAT:
        case SF_FORMAT_DOUBLE: {
            wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_F32);
            break;
        }
        default: {
            wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);
            break;
        }
        }
        wave.setSampleRate((int32_t)snd_info.samplerate);
        wave.setChannels((int32_t)snd_info.channels);
        wave.setFrameCount(snd_info.frames);
        return true;
    }
    return false;
}

bool SNDFILE_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
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
        positionInFrames = 0;

    auto sndMusic = (SNDFILE_Music*)wave->ctxData;

    return sf_seek(sndMusic->file, positionInFrames, SF_SEEK_SET) != -1;
}

int64_t SNDFILE_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto sndMusic = (SNDFILE_Music*)wave->ctxData;

    if (wave->sampleFormat == RAUDIO2_SAMPLE_FORMAT_S16)
    {
        return sf_readf_short(sndMusic->file, (short*)bufferOut, framesToRead);
    }
    else
        return sf_readf_float(sndMusic->file, (float*)bufferOut, framesToRead);
}

bool SNDFILE_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    auto sndMusic = (SNDFILE_Music*)wave->ctxData;

    if (sndMusic->file)
        sf_close(sndMusic->file);

    delete sndMusic;
    wave->ctxData = nullptr;

    return true;
}

static bool SNDFILE_getFileFormat(int format, RAudio2_Value& valueOut)
{
    auto subType = format & SF_FORMAT_SUBMASK;
    switch (subType)
    {
    case SF_FORMAT_VORBIS: {
        return ra::MakeValue("OGG", valueOut);
    }
    case SF_FORMAT_OPUS: {
        return ra::MakeValue("OPUS", valueOut);
    }
    case SF_FORMAT_ALAC_16:
    case SF_FORMAT_ALAC_20:
    case SF_FORMAT_ALAC_24:
    case SF_FORMAT_ALAC_32: {
        return ra::MakeValue("ALAC", valueOut);
    }
    case SF_FORMAT_MPEG_LAYER_I: {
        return ra::MakeValue("MP1", valueOut);
    }
    case SF_FORMAT_MPEG_LAYER_II: {
        return ra::MakeValue("MP2", valueOut);
    }
    case SF_FORMAT_MPEG_LAYER_III: {
        return ra::MakeValue("MP3", valueOut);
    }
    default:
        break;
    }

    auto type = format & SF_FORMAT_TYPEMASK;
    switch (type)
    {
    case SF_FORMAT_WAV:
    case SF_FORMAT_W64:
    case SF_FORMAT_WAVEX:
    case SF_FORMAT_WVE:
    case SF_FORMAT_RF64: {
        return ra::MakeValue("WAVE", valueOut);
    }
    case SF_FORMAT_AIFF: {
        return ra::MakeValue("AIFF", valueOut);
    }
    case SF_FORMAT_AU: {
        return ra::MakeValue("AU", valueOut);
    }
    case SF_FORMAT_RAW: {
        return ra::MakeValue("RAW", valueOut);
    }
    case SF_FORMAT_PAF: {
        return ra::MakeValue("PAF", valueOut);
    }
    case SF_FORMAT_SVX: {
        return ra::MakeValue("SVX", valueOut);
    }
    case SF_FORMAT_NIST: {
        return ra::MakeValue("NIST", valueOut);
    }
    case SF_FORMAT_VOC: {
        return ra::MakeValue("VOC", valueOut);
    }
    case SF_FORMAT_IRCAM: {
        return ra::MakeValue("IRCAM", valueOut);
    }
    case SF_FORMAT_MAT4: {
        return ra::MakeValue("MAT4", valueOut);
    }
    case SF_FORMAT_MAT5: {
        return ra::MakeValue("MAT5", valueOut);
    }
    case SF_FORMAT_PVF: {
        return ra::MakeValue("PVF", valueOut);
    }
    case SF_FORMAT_XI: {
        return ra::MakeValue("XI", valueOut);
    }
    case SF_FORMAT_HTK: {
        return ra::MakeValue("HTK", valueOut);
    }
    case SF_FORMAT_SDS: {
        return ra::MakeValue("SDS", valueOut);
    }
    case SF_FORMAT_AVR: {
        return ra::MakeValue("AVR", valueOut);
    }
    case SF_FORMAT_SD2: {
        return ra::MakeValue("SD2", valueOut);
    }
    case SF_FORMAT_FLAC: {
        return ra::MakeValue("FLAC", valueOut);
    }
    case SF_FORMAT_CAF: {
        return ra::MakeValue("CAF", valueOut);
    }
    case SF_FORMAT_OGG: {
        return ra::MakeValue("OGG", valueOut);
    }
    case SF_FORMAT_MPC2K: {
        return ra::MakeValue("MPC2K", valueOut);
    }
    case SF_FORMAT_MPEG: {
        return ra::MakeValue("MPEG", valueOut);
    }
    default:
        break;
    }
    valueOut = {};
    return false;
}

bool SNDFILE_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("SndFile"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 12> extensions{
            ".alac", ".flac", ".mp3", ".mp2", ".mp1", ".ogg",
            ".opus", ".wav", ".wave", ".aif", ".aiff", nullptr
        };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto sndMusic = (SNDFILE_Music*)wave->ctxData;
        if (!sndMusic || !sndMusic->file)
            break;

        switch (keyHash)
        {
        case ra::str2int("album"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_ALBUM), *valueOut);
        }
        case ra::str2int("artist"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_ARTIST), *valueOut);
        }
        case ra::str2int("comment"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_COMMENT), *valueOut);
        }
        case ra::str2int("copyright"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_COPYRIGHT), *valueOut);
        }
        case ra::str2int("date"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_DATE), *valueOut);
        }
        case ra::str2int("genre"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_GENRE), *valueOut);
        }
        case ra::str2int("license"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_LICENSE), *valueOut);
        }
        case ra::str2int("software"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_SOFTWARE), *valueOut);
        }
        case ra::str2int("title"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_TITLE), *valueOut);
        }
        case ra::str2int("tracknumber"): {
            return ra::MakeValue(sf_get_string(sndMusic->file, SF_STR_TRACKNUMBER), *valueOut);
        }
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            auto val = (int64_t)sf_current_byterate(sndMusic->file);
            return ra::MakeValue(val > 0 ? val * 8 : 0, *valueOut);
        }
        case ra::str2int("format"): {
            return SNDFILE_getFileFormat(sndMusic->info.format, *valueOut);
        }
        case ra::str2int("vbr"): {
            auto val = sf_command(sndMusic->file, SFC_GET_BITRATE_MODE, nullptr, 0) == SF_BITRATE_MODE_VARIABLE;
            return ra::MakeValue(val, *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
