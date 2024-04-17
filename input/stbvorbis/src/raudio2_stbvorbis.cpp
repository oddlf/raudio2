#include "raudio2_stbvorbis.h"
#include <array>
#include <memory>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"
#include <vector>

using namespace std::literals;

// TODO: Remap stb_vorbis malloc()/free() calls to RAUDIO2_MALLOC/RAUDIO2_FREE
#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.c" // OGG loading functions

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return STBVORBIS_MakeInputPlugin(plugin);
}
#endif

bool STBVORBIS_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = STBVORBIS_Open;
    plugin->seek = STBVORBIS_Seek;
    plugin->read = STBVORBIS_Read;
    plugin->close = STBVORBIS_Close;
    plugin->getValue = STBVORBIS_GetValue;

    return true;
}

struct STBVORBIS_Music {
    stb_vorbis* file{ nullptr };
    std::vector<unsigned char> fileBytes;
};

bool STBVORBIS_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    char headerBytes[4]{};
    file.read(headerBytes, 4);

    if (headerBytes[0] != 'O' && headerBytes[1] != 'g' && headerBytes[2] != 'g' && headerBytes[3] != 'S')
        return false;

    auto vorbisMusic = std::make_unique<STBVORBIS_Music>();
    if (!vorbisMusic)
        return false;

    file.seek(0);

    auto fileSize = file.getSize();
    vorbisMusic->fileBytes.resize((std::size_t)fileSize);

    file.read(vorbisMusic->fileBytes.data(), fileSize);

    auto vorbisFile = stb_vorbis_open_memory(vorbisMusic->fileBytes.data(), (int)fileSize, NULL, NULL);

    if (vorbisFile)
    {
        vorbisMusic->file = vorbisFile;

        wave.setCtxData(vorbisMusic.release());

        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);

        stb_vorbis_info info = stb_vorbis_get_info(vorbisFile);
        wave.setSampleRate((int32_t)info.sample_rate);
        //wave.setChannels((int32_t)info.channels);
        wave.setChannels(2);

        // WARNING: It seems this function returns length in frames, not samples, so we multiply by channels
        wave.setFrameCount((int64_t)stb_vorbis_stream_length_in_samples(vorbisFile));

        return true;
    }
    return false;
}

bool STBVORBIS_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
{
    if (!wave)
        return false;
    if (!wave->file)
        return false;
    if (!wave->file->handle)
        return false;
    if (!wave->ctxData)
        return false;

    auto music = (STBVORBIS_Music*)wave->ctxData;

    if (positionInFrames <= 0)
        return stb_vorbis_seek_start(music->file) == TRUE;

    return stb_vorbis_seek_frame(music->file, (unsigned int)positionInFrames) == TRUE;
}

int64_t STBVORBIS_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto music = (STBVORBIS_Music*)wave->ctxData;
    return stb_vorbis_get_samples_short_interleaved(music->file, music->file->channels, (short*)bufferOut, (int)(framesToRead * music->file->channels));
}

bool STBVORBIS_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    auto music = (STBVORBIS_Music*)wave->ctxData;
    stb_vorbis_close(music->file);
    delete music;
    wave->ctxData = nullptr;
    return true;
}

bool STBVORBIS_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    // only process keys with less than 32 chars
    switch (ra::str2int(std::string_view(key, keyLength).substr(0, 32)))
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("stb_vorbis"sv, *valueOut);
        return true;
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 7> extensions{ ".ogg", ".oga", ".ogm", ".ogv", ".ogx", ".spx", nullptr };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        break;
    }
    }
    *valueOut = {};
    return false;
}
