#include "raudio2_gme.h"
#include <gme/gme.h>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"
#include <vector>

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return GME_MakeInputPlugin(plugin);
}
#endif

bool GME_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->open = GME_Open;
    plugin->seek = GME_Seek;
    plugin->read = GME_Read;
    plugin->close = GME_Close;
    plugin->getValue = GME_GetValue;

    return true;
}

struct GME_Music {
    Music_Emu* music{ nullptr };
    gme_info_t* info{ nullptr };
    const char* format{ nullptr };
};

bool GME_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    auto fileSize = file.getSize();

    std::vector<unsigned char> fileBytes;
    fileBytes.resize(4);
    if (file.read(fileBytes.data(), 4) != 4)
        return false;

    const char* format = nullptr;
    if (!(format = gme_identify_header(fileBytes.data())))
        return false;

    auto file_type = gme_identify_extension(format);
    if (!file_type)
        return false;

    fileBytes.resize((size_t)fileSize);
    file.read(fileBytes.data() + 4, fileSize - 4);

    Music_Emu* music = nullptr;
    if (!(music = gme_new_emu(file_type, 44100)))
        return false;

    auto err = gme_load_data(music, fileBytes.data(), (long)fileBytes.size());
    if (err)
        return false;

    int count = gme_track_count(music);

    if (count < 1)
        return false;

    gme_start_track(music, 0);
    gme_info_t* track_info;
    err = gme_track_info(music, &track_info, 0);
    if (err)
        return false;

    if (music)
    {
        auto gmeMusic = new GME_Music();
        if (!gmeMusic)
            return false;

        gmeMusic->music = music;
        gmeMusic->info = track_info;
        gmeMusic->format = format;

        wave.setCtxData(gmeMusic);

        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);
        wave.setSampleRate(44100);
        wave.setChannels(2);
        wave.setFrameCount((int64_t)(((float)(track_info->intro_length + track_info->loop_length) / 1000.f) * 44100.f * 2.f));
        return true;
    }

    return false;
}

bool GME_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames)
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

    auto music = (GME_Music*)wave->ctxData;

    return gme_seek(music->music, (int)(positionInFrames / (wave->sampleRate * wave->channels))) == nullptr;
}

int64_t GME_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead)
{
    if (!wave)
        return 0;
    if (!wave->file)
        return 0;
    if (!wave->file->handle)
        return 0;
    if (!wave->ctxData)
        return 0;

    auto music = (GME_Music*)wave->ctxData;

    if (gme_track_ended(music->music))
        return 0;

    if (gme_play(music->music, (int)framesToRead * 2, (short*)bufferOut))
        return 0;

    return framesToRead;
}

bool GME_Close(RAudio2_WaveInfo* wave)
{
    if (!wave)
        return false;
    if (!wave->ctxData)
        return false;

    auto gmeMusic = (GME_Music*)wave->ctxData;

    if (gmeMusic->info)
        gme_free_info(gmeMusic->info);

    if (gmeMusic->music)
        gme_delete(gmeMusic->music);

    delete gmeMusic;
    wave->ctxData = nullptr;

    return true;
}

bool GME_GetValue(RAudio2_WaveInfo* wave_, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    ra::WaveInfo wave = wave_;

    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("Game_Music_Emu"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 12> extensions{
            ".vgm", ".vgz", ".ay", ".gbs", ".gym", ".hes",
            ".kss", ".nsf", ".nsfe", ".sap", ".spc", nullptr
        };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto gmeMusic = (GME_Music*)wave.getCtxData();
        if (!gmeMusic || !gmeMusic->info)
            break;

        switch (keyHash)
        {
        case ra::str2int("artist"):
        case ra::str2int("author"): {
            return ra::MakeValue(gmeMusic->info->author, *valueOut);
        }
        case ra::str2int("comment"): {
            return ra::MakeValue(gmeMusic->info->comment, *valueOut);
        }
        case ra::str2int("copyright"): {
            return ra::MakeValue(gmeMusic->info->copyright, *valueOut);
        }
        case ra::str2int("dumper"): {
            return ra::MakeValue(gmeMusic->info->dumper, *valueOut);
        }
        case ra::str2int("game"): {
            return ra::MakeValue(gmeMusic->info->game, *valueOut);
        }
        case ra::str2int("song"):
        case ra::str2int("title"): {
            return ra::MakeValue(gmeMusic->info->song, *valueOut);
        }
        case ra::str2int("system"): {
            return ra::MakeValue(gmeMusic->info->system, *valueOut);
        }
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            return ra::MakeValue(wave.calculateBps(), *valueOut);
        }
        case ra::str2int("format"): {
            return ra::MakeValue(gmeMusic->format, *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
