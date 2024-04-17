#include "raudio2_modplug.h"
#include <array>
#include <libmodplug/modplug.h>
#include "raudio2/raudio2_common.hpp"
#include "raudio2/raudio2_value.hpp"
#include "raudio2/raudio2_waveinfo.hpp"
#include <vector>

using namespace std::literals;

static ModPlug_Settings modplugSettings{ 0 };
static unsigned int modplugMasterVolume{ 512 };

#ifdef RAUDIO2_STANDALONE_PLUGIN
bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin)
{
    return MODPLUG_MakeInputPlugin(plugin);
}
#endif

bool MODPLUG_MakeInputPlugin(RAudio2_InputPlugin* plugin)
{
    plugin->flags = 0;
    plugin->init = MODPLUG_Init;
    plugin->open = MODPLUG_Open;
    plugin->seek = MODPLUG_Seek;
    plugin->read = MODPLUG_Read;
    plugin->close = MODPLUG_Close;
    plugin->getValue = MODPLUG_GetValue;

    return true;
}

bool MODPLUG_Init()
{
    ModPlug_GetSettings(&modplugSettings);
    modplugSettings.mLoopCount = 1;
    ModPlug_SetSettings(&modplugSettings);
    return true;
}

bool MODPLUG_Open(RAudio2_WaveInfo* wave_)
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

    auto modFile = ModPlug_Load(fileBytes.data(), (int)fileBytes.size());
    if (!modFile)
    {
        return false;
    }

    ModPlug_SetMasterVolume(modFile, modplugMasterVolume);

    wave.setCtxData(modFile);

    switch (modplugSettings.mBits)
    {
    case 8: {
        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_U8);
        break;
    }
    case 32: {
        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S32);
        break;
    }
    default: {
        wave.setSampleFormat(RAUDIO2_SAMPLE_FORMAT_S16);
        break;
    }
    }
    wave.setSampleRate(modplugSettings.mFrequency);
    wave.setChannels(modplugSettings.mChannels);

    auto numFrames = (int32_t)(((float)(ModPlug_GetLength(modFile) / 1000.f)) * (float)modplugSettings.mFrequency * (float)modplugSettings.mChannels);
    wave.setFrameCount(numFrames);
    return true;
}

bool MODPLUG_Seek(RAudio2_WaveInfo* wave_, int64_t positionInFrames)
{
    ra::WaveInfo wave = wave_;
    if (!wave)
        return false;

    if (positionInFrames <= 0)
        positionInFrames = 0;

    ModPlug_Seek((ModPlugFile*)wave.getCtxData(), (int)wave.framesToMilliseconds(positionInFrames));

    return true;
}

int64_t MODPLUG_Read(RAudio2_WaveInfo* wave_, void* bufferOut, int64_t framesToRead)
{
    ra::WaveInfo wave = wave_;
    if (!wave)
        return 0;

    int multiplier = wave_->channels;
    switch (wave_->sampleFormat)
    {
    case RAUDIO2_SAMPLE_FORMAT_U8: {
        multiplier *= 1;
        break;
    }
    case RAUDIO2_SAMPLE_FORMAT_S32: {
        multiplier *= 4;
        break;
    }
    default: {
        multiplier *= 2;
        break;
    }
    }

    int samplesRead = ModPlug_Read((ModPlugFile*)wave.getCtxData(), bufferOut, (int)(framesToRead * multiplier));
    return samplesRead / multiplier;
}

bool MODPLUG_Close(RAudio2_WaveInfo* wave_)
{
    ra::WaveInfo wave = wave_;
    if (!wave.hasValidCtxData())
        return false;

    ModPlug_Unload((ModPlugFile*)wave.getCtxData());
    wave.setCtxData(nullptr);
    return true;
}

static bool MODPLUG_getFileFormat(int type, RAudio2_Value& valueOut)
{
    constexpr int MOD_TYPE_MOD = 0x01;
    constexpr int MOD_TYPE_S3M = 0x02;
    constexpr int MOD_TYPE_XM = 0x04;
    constexpr int MOD_TYPE_MED = 0x08;
    constexpr int MOD_TYPE_MTM = 0x10;
    constexpr int MOD_TYPE_IT = 0x20;
    constexpr int MOD_TYPE_669 = 0x40;
    constexpr int MOD_TYPE_ULT = 0x80;
    constexpr int MOD_TYPE_STM = 0x100;
    constexpr int MOD_TYPE_FAR = 0x200;
    constexpr int MOD_TYPE_WAV = 0x400;
    constexpr int MOD_TYPE_AMF = 0x800;
    constexpr int MOD_TYPE_AMS = 0x1000;
    constexpr int MOD_TYPE_DSM = 0x2000;
    constexpr int MOD_TYPE_MDL = 0x4000;
    constexpr int MOD_TYPE_OKT = 0x8000;
    constexpr int MOD_TYPE_MID = 0x10000;
    constexpr int MOD_TYPE_DMF = 0x20000;
    constexpr int MOD_TYPE_PTM = 0x40000;
    constexpr int MOD_TYPE_DBM = 0x80000;
    constexpr int MOD_TYPE_MT2 = 0x100000;
    constexpr int MOD_TYPE_AMF0 = 0x200000;
    constexpr int MOD_TYPE_PSM = 0x400000;
    constexpr int MOD_TYPE_J2B = 0x800000;
    constexpr int MOD_TYPE_ABC = 0x1000000;
    constexpr int MOD_TYPE_PAT = 0x2000000;
    constexpr int MOD_TYPE_UMX = 0x80000000;

    switch (type)
    {
    case MOD_TYPE_MOD: {
        return ra::MakeValue("MOD", valueOut);
    }
    case MOD_TYPE_S3M: {
        return ra::MakeValue("S3M", valueOut);
    }
    case MOD_TYPE_XM: {
        return ra::MakeValue("XM", valueOut);
    }
    case MOD_TYPE_MED: {
        return ra::MakeValue("MED", valueOut);
    }
    case MOD_TYPE_MTM: {
        return ra::MakeValue("MTM", valueOut);
    }
    case MOD_TYPE_IT: {
        return ra::MakeValue("IT", valueOut);
    }
    case MOD_TYPE_669: {
        return ra::MakeValue("669", valueOut);
    }
    case MOD_TYPE_ULT: {
        return ra::MakeValue("ULT", valueOut);
    }
    case MOD_TYPE_STM: {
        return ra::MakeValue("STM", valueOut);
    }
    case MOD_TYPE_FAR: {
        return ra::MakeValue("FAR", valueOut);
    }
    case MOD_TYPE_WAV: {
        return ra::MakeValue("WAV", valueOut);
    }
    case MOD_TYPE_AMF: {
        return ra::MakeValue("AMF", valueOut);
    }
    case MOD_TYPE_AMS: {
        return ra::MakeValue("AMS", valueOut);
    }
    case MOD_TYPE_DSM: {
        return ra::MakeValue("DSM", valueOut);
    }
    case MOD_TYPE_MDL: {
        return ra::MakeValue("MDL", valueOut);
    }
    case MOD_TYPE_OKT: {
        return ra::MakeValue("OKT", valueOut);
    }
    case MOD_TYPE_MID: {
        return ra::MakeValue("MID", valueOut);
    }
    case MOD_TYPE_DMF: {
        return ra::MakeValue("DMF", valueOut);
    }
    case MOD_TYPE_PTM: {
        return ra::MakeValue("PTM", valueOut);
    }
    case MOD_TYPE_DBM: {
        return ra::MakeValue("DBM", valueOut);
    }
    case MOD_TYPE_MT2: {
        return ra::MakeValue("MT2", valueOut);
    }
    case MOD_TYPE_AMF0: {
        return ra::MakeValue("AMF0", valueOut);
    }
    case MOD_TYPE_PSM: {
        return ra::MakeValue("PSM", valueOut);
    }
    case MOD_TYPE_J2B: {
        return ra::MakeValue("J2B", valueOut);
    }
    case MOD_TYPE_ABC: {
        return ra::MakeValue("ABC", valueOut);
    }
    case MOD_TYPE_PAT: {
        return ra::MakeValue("PAT", valueOut);
    }
    case MOD_TYPE_UMX: {
        return ra::MakeValue("UMX", valueOut);
    }
    default:
        break;
    }
    valueOut = {};
    return false;
}

bool MODPLUG_GetValue(RAudio2_WaveInfo* wave_, const char* key, int32_t keyLength, RAudio2_Value* valueOut)
{
    ra::WaveInfo wave = wave_;

    // only process keys with less than 32 chars
    auto keyHash = ra::str2int(std::string_view(key, keyLength).substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("plugin_name"): {
        return ra::MakeValue("modplug"sv, *valueOut);
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
        auto modFile = (ModPlugFile*)wave.getCtxData();
        if (!modFile)
            break;

        switch (keyHash)
        {
        case ra::str2int("name"):
        case ra::str2int("title"): {
            return ra::MakeValue(ModPlug_GetName(modFile), *valueOut);
        }
        case ra::str2int("bps"):
        case ra::str2int("current_bps"): {
            return ra::MakeValue(wave.calculateBps(), *valueOut);
        }
        case ra::str2int("format"): {
            return MODPLUG_getFileFormat(ModPlug_GetModuleType(modFile), *valueOut);
        }
        default:
            break;
        }
    }
    }
    *valueOut = {};
    return false;
}
