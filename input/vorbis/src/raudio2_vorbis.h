#pragma once

#include "raudio2/raudio2_config.h"
#include "raudio2/raudio2_inputplugin.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RAUDIO2_STANDALONE_PLUGIN
RAUDIO2_EXPORT bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin);
#endif

bool VORBIS_MakeInputPlugin(RAudio2_InputPlugin* plugin);

bool VORBIS_Init();

bool VORBIS_Open(RAudio2_WaveInfo* wave);

int64_t VORBIS_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead);

bool VORBIS_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames);

bool VORBIS_Close(RAudio2_WaveInfo* wave);

bool VORBIS_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

#ifdef __cplusplus
}
#endif
