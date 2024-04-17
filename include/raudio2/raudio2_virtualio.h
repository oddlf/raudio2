#ifndef RAUDIO2_IO_H
#define RAUDIO2_IO_H

#include <stdint.h>

#define RAUDIO2_SEEK_SET 0
#define RAUDIO2_SEEK_CUR 1
#define RAUDIO2_SEEK_END 2

typedef int64_t (*RAudio2_VirtualIO_IORead)(void* handle, void* bufferOut, int64_t bytesToRead);
typedef int64_t (*RAudio2_VirtualIO_IOWrite)(void* handle, const void* bufferIn, int64_t bytesToWrite);
typedef int64_t (*RAudio2_VirtualIO_IOSeek)(void* handle, int64_t offset, int whence);
typedef int64_t (*RAudio2_VirtualIO_IOTell)(void* handle);
typedef int64_t (*RAudio2_VirtualIO_IOGetSize)(void* handle);

typedef struct RAudio2_VirtualIO {
    void* handle;
    RAudio2_VirtualIO_IORead read;
    RAudio2_VirtualIO_IOWrite write;
    RAudio2_VirtualIO_IOSeek seek;
    RAudio2_VirtualIO_IOTell tell;
    RAudio2_VirtualIO_IOGetSize getSize;

#ifdef __cplusplus
    RAudio2_VirtualIO() : handle{}, read{}, write{}, seek{}, tell{}, getSize{}
    {
    }
#endif
} RAudio2_VirtualIO;

#endif // RAUDIO2_IO_H
