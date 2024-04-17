#pragma once

#include "raudio2/raudio2_config.h"
#include "raudio2/raudio2_inputplugin.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RAUDIO2_STANDALONE_PLUGIN
RAUDIO2_EXPORT bool RAudio2_GetInputPlugin(RAudio2_InputPlugin* plugin);
#endif

bool OPENMPT_MakeInputPlugin(RAudio2_InputPlugin* plugin);

bool OPENMPT_Init();

bool OPENMPT_Open(RAudio2_WaveInfo* wave);

int64_t OPENMPT_Read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead);

bool OPENMPT_Seek(RAudio2_WaveInfo* wave, int64_t positionInFrames);

bool OPENMPT_Close(RAudio2_WaveInfo* wave);

bool OPENMPT_GetValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

#ifdef __cplusplus
}
#endif
