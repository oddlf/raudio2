#include "raudio2_openmpt.h"
#include <array>
#include <libopenmpt/libopenmpt.h>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"
#include <vector>

using namespace std::literals;

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return OPENMPT_MakeInputPlugin(plugin);
}
#endif

bool OPENMPT_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->init = OPENMPT_Init;
    plugin->open = OPENMPT_Open;
    plugin->seek = OPENMPT_Seek;
    plugin->read = OPENMPT_Read;
    plugin->close = OPENMPT_Close;
    plugin->getValue = OPENMPT_GetValue;

    return true;
}

bool OPENMPT_Init()
{
    return true;
}

bool OPENMPT_Open(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidWave())
        return false;

    auto file = wave.getFile();

    if (!file)
        return false;

    auto fileSize = file.getSize();

    std::vector<unsigned char> fileBytes;
    fileBytes.resize((std::size_t)fileSize);

    file.read(fileBytes.data(), fileSize);

    auto modFile = openmpt_module_create_from_memory2(fileBytes.data(), fileBytes.size(),
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!modFile)
        return false;

    wave.setCtxData(modFile);

    wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_F32);
    wave.setSampleRate(48000);
    wave.setChannels(2);

    auto numFrames = (uint32_t)(openmpt_module_get_duration_seconds(modFile) * 48000.0);
    wave.setFrameCount(numFrames);

    return true;
}

bool OPENMPT_Seek(RAudio2_WaveInfo* wave_, int64_t positionInFrames)
{
    ra::WaveInfo wave = wave_;
    if (!wave)
        return false;

    if (positionInFrames <= 0)
        positionInFrames = 0;

    auto modFile = (openmpt_module*)wave.getCtxData();

    openmpt_module_set_position_seconds(modFile, wave.framesToSeconds(positionInFrames));

    return true;
}

int64_t OPENMPT_Read(RAudio2_WaveInfo* wave_, void* bufferOut, int64_t framesToRead)
{
    ra::WaveInfo wave = wave_;
    if (!wave)
        return 0;

    auto modFile = (openmpt_module*)wave.getCtxData();

    int samplesRead = openmpt_module_read_interleaved_float_stereo(modFile, wave.getSampleRate(), (size_t)framesToRead, (float*)bufferOut);
    return samplesRead;
}

bool OPENMPT_Close(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidCtxData())
        return false;

    openmpt_module_destroy((openmpt_module*)wave.getCtxData());
    wave.setCtxData(nullptr);
    return true;
}

bool OPENMPT_GetValue(RAudio2_WaveInfo* wave_, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    ra::WaveInfo wave = wave_;

    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("openmpt"sv, *valueOut);
    }
    case ra::str2int("plugin_extensions"): {
        static const std::array<const char*, 27> extensions{
            ".mod", ".xm", ".it", ".669", ".abc", ".amf", ".amf0", ".ams", ".dbm",
            ".dmf", ".dsm", ".far", ".j2b", ".mdl", ".med", ".mid", ".mt2", ".mtm",
            ".okt", ".pat", ".psm", ".ptm", ".s3m", ".stm", ".ult", ".umx", nullptr
        };
        return ra::MakeArrayValue(extensions, *valueOut);
    }
    default: {
        auto modFile = (openmpt_module*)wave.getCtxData();
        if (!modFile)
            break;

        switch (keyHash)
        {
        case ra::str2int("artist"): {
            return ra::MakeValue(openmpt_module_get_metadata(modFile, "artist"), *valueOut);
        }
        case ra::str2int("date"): {
            return ra::MakeValue(openmpt_module_get_metadata(modFile, "date"), *valueOut);
        }
        case ra::str2int("comment"):
        case ra::str2int("message"): {
            return ra::MakeValue(openmpt_module_get_metadata(modFile, "message"), *valueOut);
        }
        case ra::str2int("title"): {
            return ra::MakeValue(openmpt_module_get_metadata(modFile, "title"), *valueOut);
        }
        case ra::str2int("tracker"): {
            return ra::MakeValue(openmpt_module_get_metadata(modFile, "tracker"), *valueOut);
        }
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            return ra::MakeValue(wave.calculateBps(), *valueOut);
        }
        case ra::str2int("format"):
        case ra::str2int("type"): {
            return ra::MakeValue(openmpt_module_get_metadata(modFile, "type"), *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
