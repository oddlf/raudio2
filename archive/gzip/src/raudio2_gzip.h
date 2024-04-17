#pragma once

#include "raudio2/raudio2_archiveplugin.hpp"
#include "raudio2/raudio2_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RAUDIO2_STANDALONE_PLUGIN
RAUDIO2_EXPORT bool RAudio2_GetArchivePlugin(RAudio2_ArchivePlugin* plugin);
#endif

bool GZIP_MakeArchivePlugin(RAudio2_ArchivePlugin* plugin);

bool GZIP_ArchiveOpen(RAudio2_Archive* archive);

bool GZIP_ArchiveClose(RAudio2_Archive* archive);

bool GZIP_FileOpen(RAudio2_Archive* archive, const char* filePath, void** fileCtxOut);

int64_t GZIP_FileRead(void* fileCtx, void* bufferOut, int64_t bytesToRead);

int64_t GZIP_FileSeek(void* fileCtx, int64_t offset, int whence);

int64_t GZIP_FileTell(void* fileCtx);

int64_t GZIP_FileGetSize(void* fileCtx);

bool GZIP_FileClose(void* fileCtx);

bool GZIP_GetValue(void* fileCtx, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

#ifdef __cplusplus
}
#endif
