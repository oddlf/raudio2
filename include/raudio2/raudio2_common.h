#ifndef RAUDIO2_COMMON_H
#define RAUDIO2_COMMON_H

typedef void* RAUDIO2_HANDLE;

typedef enum
{
    RAUDIO2_FLAG_NONE,
    RAUDIO2_FLAG_AUTOUPDATE
} RAudio2_Flags;

typedef enum
{
    RAUDIO2_PLUGIN_ANY,
    RAUDIO2_PLUGIN_ARCHIVE,
    RAUDIO2_PLUGIN_INPUT
} RAudio2_PluginType;

// Audio sample formats
// NOTE: Values must match miniaudio's ma_format
typedef enum
{
    RAUDIO2_SAMPLE_FORMAT_UNKNOWN,
    RAUDIO2_SAMPLE_FORMAT_U8,
    RAUDIO2_SAMPLE_FORMAT_S16,
    RAUDIO2_SAMPLE_FORMAT_S24,
    RAUDIO2_SAMPLE_FORMAT_S32,
    RAUDIO2_SAMPLE_FORMAT_F32,
    RAUDIO2_SAMPLE_FORMAT_COUNT
} RAudio2_SampleFormat;

// Trace log level
// NOTE: Organized by priority level
typedef enum
{
    RAUDIO2_LOG_ALL,     // Display all logs
    RAUDIO2_LOG_TRACE,   // Trace logging, intended for internal use only
    RAUDIO2_LOG_DEBUG,   // Debug logging, used for internal debugging, it should be disabled on release builds
    RAUDIO2_LOG_INFO,    // Info logging, used for program execution info
    RAUDIO2_LOG_WARNING, // Warning logging, used on recoverable failures
    RAUDIO2_LOG_ERROR,   // Error logging, used on unrecoverable failures
    RAUDIO2_LOG_FATAL,   // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    RAUDIO2_LOG_NONE     // Disable logging
} RAudio2_TraceLogLevel;

#endif // RAUDIO2_COMMON_H
