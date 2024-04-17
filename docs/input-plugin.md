# input plugin API

Input plugins decode audio formats.

The input plugin API is stable and shouldn't change the public interface.

## API

### Value

`Value` stores a value (64 bit integer, double, string, pointer or array of strings)

```c
typedef enum
{
    RAUDIO2_VALUE_NONE,
    RAUDIO2_VALUE_INT64,
    RAUDIO2_VALUE_DOUBLE,
    RAUDIO2_VALUE_STRING,
    RAUDIO2_VALUE_POINTER,
    RAUDIO2_VALUE_COUNT
} RAudio2_ValueType;

typedef struct RAudio2_Value {
    union {
        int64_t num;
        double numf;
        const char* str;
        void* ptr;
    } value;
    uint32_t size;
    int32_t type;
} RAudio2_Value;
```

### VirtualIO

`VirtualIO` Abstracts IO and should be used to read files or streams (no seek) in input plugins

```c
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
} RAudio2_VirtualIO;
```

### WaveInfo

`VirtualIO` Stores information on the audio file and should be filled by the input plugin's open function  
`file` is provided for the input plugin to read data

```c
typedef struct RAudio2_WaveInfo {
    int32_t sampleFormat;    // Sample format (RAudio2_SampleFormat, signed 16 bit, float 32 bit, ...)
    int32_t sampleRate;      // Frequency (samples per second)
    int32_t channels;        // Number of channels (1-mono, 2-stereo, ...)
    int64_t frameCount;      // Total number of frames (considering channels)
    RAudio2_VirtualIO* file; // Audio file
    void* ctxData;           // Audio context data, depends on type
} RAudio2_WaveInfo;
```

### InputPlugin

`InputPlugin` Defines functions to open/read/seek/close an audio file  
`WaveInfo` is provided for the input plugin to read data and store audio information

To make an input plugin, export the `RAudio2_InputPlugin` function, which will be called by raudio2

```c
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
} RAudio2_InputPlugin;

typedef bool (*RAudio2_GetInputPluginFunc)(RAudio2_InputPlugin*);
```
