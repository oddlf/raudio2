#ifndef RAUDIO2_ARCHIVEPLUGIN_H
#define RAUDIO2_ARCHIVEPLUGIN_H

#include "raudio2_archive.h"
#include "raudio2_value.h"
#include <stdbool.h>

typedef bool (*RAudio2_ArchivePlugin_InitFunc)();
typedef bool (*RAudio2_ArchivePlugin_UninitFunc)();
typedef bool (*RAudio2_ArchivePlugin_ArchiveOpenFunc)(RAudio2_Archive* archive);
typedef bool (*RAudio2_ArchivePlugin_ArchiveCloseFunc)(RAudio2_Archive* archive);
typedef bool (*RAudio2_ArchivePlugin_FileOpenFunc)(RAudio2_Archive* archive, const char* filePath, void** fileCtxOut);
typedef int64_t (*RAudio2_ArchivePlugin_FileReadFunc)(void* fileCtx, void* bufferOut, int64_t bytesToRead);
typedef int64_t (*RAudio2_ArchivePlugin_FileSeekFunc)(void* fileCtx, int64_t offset, int whence);
typedef int64_t (*RAudio2_ArchivePlugin_FileTellFunc)(void* fileCtx);
typedef int64_t (*RAudio2_ArchivePlugin_FileGetSizeFunc)(void* fileCtx);
typedef bool (*RAudio2_ArchivePlugin_FileCloseFunc)(void* fileCtx);
typedef bool (*RAudio2_ArchivePlugin_GetValueFunc)(void* fileCtx, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

typedef struct RAudio2_ArchivePlugin {
    int32_t flags;                           // should always be 0
    RAudio2_ArchivePlugin_InitFunc init;     // optional
    RAudio2_ArchivePlugin_UninitFunc uninit; // optional
    RAudio2_ArchivePlugin_ArchiveOpenFunc archiveOpen;
    RAudio2_ArchivePlugin_ArchiveCloseFunc archiveClose;
    RAudio2_ArchivePlugin_FileOpenFunc fileOpen;
    RAudio2_ArchivePlugin_FileReadFunc fileRead;
    RAudio2_ArchivePlugin_FileSeekFunc fileSeek;
    RAudio2_ArchivePlugin_FileTellFunc fileTell;
    RAudio2_ArchivePlugin_FileGetSizeFunc fileGetSize;
    RAudio2_ArchivePlugin_FileCloseFunc fileClose;
    RAudio2_ArchivePlugin_GetValueFunc getValue;

#ifdef __cplusplus
    RAudio2_ArchivePlugin() : flags{}, init{}, uninit{}, archiveOpen{}, archiveClose{}, fileOpen{},
                              fileRead{}, fileSeek{}, fileTell{}, fileGetSize{}, fileClose{}, getValue{}
    {
    }
#endif
} RAudio2_ArchivePlugin;

typedef bool (*RAudio2_GetArchivePluginFunc)(RAudio2_ArchivePlugin*);

#endif // RAUDIO2_ARCHIVEPLUGIN_H
