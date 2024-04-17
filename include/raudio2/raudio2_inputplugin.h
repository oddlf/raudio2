#ifndef RAUDIO2_INPUTPLUGIN_H
#define RAUDIO2_INPUTPLUGIN_H

#include "raudio2_value.h"
#include "raudio2_waveinfo.h"
#include <stdbool.h>

typedef bool (*RAudio2_InputPlugin_InitFunc)();
typedef bool (*RAudio2_InputPlugin_UninitFunc)();
typedef bool (*RAudio2_InputPlugin_OpenFunc)(RAudio2_WaveInfo* wave);
typedef int64_t (*RAudio2_InputPlugin_ReadFunc)(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead);
typedef bool (*RAudio2_InputPlugin_SeekFunc)(RAudio2_WaveInfo* wave, int64_t positionInFrames);
typedef bool (*RAudio2_InputPlugin_CloseFunc)(RAudio2_WaveInfo* wave);
typedef bool (*RAudio2_InputPlugin_GetValueFunc)(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

typedef struct RAudio2_InputPlugin {
    int32_t flags;                         // should always be 0
    RAudio2_InputPlugin_InitFunc init;     // optional
    RAudio2_InputPlugin_UninitFunc uninit; // optional
    RAudio2_InputPlugin_OpenFunc open;
    RAudio2_InputPlugin_ReadFunc read;
    RAudio2_InputPlugin_SeekFunc seek;
    RAudio2_InputPlugin_CloseFunc close;
    RAudio2_InputPlugin_GetValueFunc getValue;

#ifdef __cplusplus
    RAudio2_InputPlugin() : flags{}, init{}, uninit{}, open{}, read{}, seek{}, close{}, getValue{}
    {
    }
#endif
} RAudio2_InputPlugin;

typedef bool (*RAudio2_GetInputPluginFunc)(RAudio2_InputPlugin*);

#endif // RAUDIO2_INPUTPLUGIN_H
