#ifndef RAUDIO2_ARCHIVE_H
#define RAUDIO2_ARCHIVE_H

#include "raudio2_virtualio.h"

typedef struct RAudio2_Archive {
    RAudio2_VirtualIO* file; // Archive file
    void* ctxData;           // Archive context data

#ifdef __cplusplus
    RAudio2_Archive() : file{}, ctxData{}
    {
    }
#endif
} RAudio2_Archive;

#endif // RAUDIO2_ARCHIVE_H
