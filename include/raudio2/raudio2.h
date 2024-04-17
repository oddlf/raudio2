/**********************************************************************************************
 *
 *   raudio2 v1.0 - A simple and easy-to-use audio library based on raudio + miniaudio
 *
 *   CONTRIBUTORS:
 *       Ramon Santamaria (github: @raysan5) (2014-2024):
 *           - raudio library
 *
 *       David Reid (github: @mackron) (Nov. 2017):
 *           - Complete port to miniaudio library
 *
 *       Joshua Reisenauer (github: @kd7tck) (2015)
 *           - Mixing channels support
 *           - Raw audio context support
 *
 *
 *   LICENSE: zlib/libpng
 *
 *   Copyright (c) 2023-2024 oddlf
 *
 *   This software is provided "as-is", without any express or implied warranty. In no event
 *   will the authors be held liable for any damages arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose, including commercial
 *   applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *     wrote the original software. If you use this software in a product, an acknowledgment
 *     in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *     as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 *
 **********************************************************************************************/

#ifndef RAUDIO2_H
#define RAUDIO2_H

#include "raudio2_common.h"
#include "raudio2_value.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef RAUDIO2_AUDIO_DEVICE_FORMAT
#define RAUDIO2_AUDIO_DEVICE_FORMAT RAUDIO2_SAMPLE_FORMAT_F32 // Device output format (float-32bit)
#endif
#ifndef RAUDIO2_AUDIO_DEVICE_SAMPLE_RATE
#define RAUDIO2_AUDIO_DEVICE_SAMPLE_RATE 0 // Device output sample rate
#endif
#ifndef RAUDIO2_AUDIO_DEVICE_CHANNELS
#define RAUDIO2_AUDIO_DEVICE_CHANNELS 2 // Device output channels: stereo
#endif

#ifndef RAUDIO2_MAX_AUDIO_BUFFER_POOL_CHANNELS
#define RAUDIO2_MAX_AUDIO_BUFFER_POOL_CHANNELS 16 // Audio pool channels
#endif

#ifndef RAUDIO2_MALLOC
#define RAUDIO2_MALLOC(sz) malloc((sz))
#endif
#ifndef RAUDIO2_CALLOC
#define RAUDIO2_CALLOC(n, sz) calloc((n), (sz))
#endif
#ifndef RAUDIO2_REALLOC
#define RAUDIO2_REALLOC(p, sz) realloc((p), (sz))
#endif
#ifndef RAUDIO2_FREE
#define RAUDIO2_FREE(p) free((p))
#endif

#ifndef RAUDIO2_TRACELOG
#ifdef NDEBUG
#define RAUDIO2_TRACELOG(...) (void)0
#else
#include <stdio.h>
#define RAUDIO2_TRACELOG(level, ...) printf(__VA_ARGS__)
#endif
#endif

#ifdef RAUDIO2_EXPORT_DLL
#include "raudio2_config.h"
#define RAUDIO2_API RAUDIO2_EXPORT
#define RAUDIO2_CALL RAUDIO2_CALLING_CONVENTION
#else
#define RAUDIO2_API
#define RAUDIO2_CALL
#endif

typedef void (*AudioCallback)(void* bufferData, int64_t frames);

#ifdef __cplusplus
extern "C" {
#endif

// Audio device management functions

// Initialize audio device and context with the default
RAUDIO2_API RAUDIO2_HANDLE RAUDIO2_CALL RAudio2_InitAudioDevice(int32_t flags);

// Initialize audio device and context
RAUDIO2_API RAUDIO2_HANDLE RAUDIO2_CALL RAudio2_InitAudioDevice2(int32_t format, int32_t sampleRate, int32_t channels, int32_t flags);

// Close the audio device and context
RAUDIO2_API void RAUDIO2_CALL RAudio2_CloseAudioDevice(RAUDIO2_HANDLE handle);

// Check if audio device has been initialized successfully
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsAudioDeviceReady(RAUDIO2_HANDLE handle);

// Register plugin (filePath=raudio2-sndfile.dll)
RAUDIO2_API bool RAUDIO2_CALL RAudio2_RegisterPlugin(RAUDIO2_HANDLE handle, const char* filePath, int32_t pluginType, bool append);

// Get audio device format
RAUDIO2_API int32_t RAUDIO2_CALL RAudio2_GetAudioDeviceFormat(RAUDIO2_HANDLE handle);

// Get audio device sample rate
RAUDIO2_API int32_t RAUDIO2_CALL RAudio2_GetAudioDeviceSampleRate(RAUDIO2_HANDLE handle);

// Get audio device channels
RAUDIO2_API int32_t RAUDIO2_CALL RAudio2_GetAudioDeviceChannels(RAUDIO2_HANDLE handle);

// Set master volume (listener) (1.0 is max level)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetMasterVolume(RAUDIO2_HANDLE handle, float volume);

// Get master volume (listener)
RAUDIO2_API float RAUDIO2_CALL RAudio2_GetMasterVolume(RAUDIO2_HANDLE handle);

// Get audio device value (key=input.wav.plugin_extensions returns the the list of supported file extensions by the wav plugin)
RAUDIO2_API bool RAUDIO2_CALL RAudio2_GetAudioDeviceValue(RAUDIO2_HANDLE handle, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

// Music management functions

// Load music stream from file
RAUDIO2_API int32_t RAUDIO2_CALL RAudio2_LoadMusic(RAUDIO2_HANDLE handle, const char* fileName, bool streamFile);

// Load music stream from memory buffer, fileType refers to extension: i.e. ".wav"
// WARNING: File extension must be provided in lower-case
RAUDIO2_API int32_t RAUDIO2_CALL RAudio2_LoadMusicFromMemory(RAUDIO2_HANDLE handle, const char* fileType, const unsigned char* dataIn, int64_t dataSize);

// Checks if a music stream is ready
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsMusicReady(RAUDIO2_HANDLE handle, int32_t musicId);

// Unload music stream
RAUDIO2_API void RAUDIO2_CALL RAudio2_UnloadMusic(RAUDIO2_HANDLE handle, int32_t musicId);

// Start music playing
RAUDIO2_API void RAUDIO2_CALL RAudio2_PlayMusic(RAUDIO2_HANDLE handle, int32_t musicId);

// Check if music is playing
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsMusicPlaying(RAUDIO2_HANDLE handle, int32_t musicId);

// Check if music is stopped
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsMusicStopped(RAUDIO2_HANDLE handle, int32_t musicId);

// Updates buffers for music streaming
RAUDIO2_API void RAUDIO2_CALL RAudio2_UpdateMusic(RAUDIO2_HANDLE handle, int32_t musicId);

// Stop music playing
RAUDIO2_API void RAUDIO2_CALL RAudio2_StopMusic(RAUDIO2_HANDLE handle, int32_t musicId);

// Pause music playing
RAUDIO2_API void RAUDIO2_CALL RAudio2_PauseMusic(RAUDIO2_HANDLE handle, int32_t musicId);

// Resume playing paused music
RAUDIO2_API void RAUDIO2_CALL RAudio2_ResumeMusic(RAUDIO2_HANDLE handle, int32_t musicId);

// Seek music to a position (in seconds)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SeekMusic(RAUDIO2_HANDLE handle, int32_t musicId, double position);

// Set volume for music (1.0 is max level)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetMusicVolume(RAUDIO2_HANDLE handle, int32_t musicId, float volume);

// Set pitch for a music (1.0 is base level)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetMusicPitch(RAUDIO2_HANDLE handle, int32_t musicId, float pitch);

// Set pan for a music (0.0 to 1.0, 0.5=center)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetMusicPan(RAUDIO2_HANDLE handle, int32_t musicId, float pan);

// Get music time length (in seconds)
RAUDIO2_API double RAUDIO2_CALL RAudio2_GetMusicTimeLength(RAUDIO2_HANDLE handle, int32_t musicId);

// Get current music time played (in seconds)
RAUDIO2_API double RAUDIO2_CALL RAudio2_GetMusicTimePlayed(RAUDIO2_HANDLE handle, int32_t musicId);

// Get music value (key=artist returns the artist if the audio format has metadata)
RAUDIO2_API bool RAUDIO2_CALL RAudio2_GetMusicValue(RAUDIO2_HANDLE handle, int32_t musicId, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

// Get music's archive value (key=archive_plugin returns the name of the archive plugin being used, if any)
RAUDIO2_API bool RAUDIO2_CALL RAudio2_GetMusicArchiveValue(RAUDIO2_HANDLE handle, int32_t musicId, const char* key, int32_t keyLength, RAudio2_Value* valueOut);

// AudioStream management functions

// Load audio stream (to stream raw audio pcm data)
RAUDIO2_API int32_t RAUDIO2_CALL RAudio2_LoadAudioStream(RAUDIO2_HANDLE handle, int32_t sampleFormat, int32_t sampleRate, int32_t channels);

// Checks if an audio stream is ready
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsAudioStreamReady(RAUDIO2_HANDLE handle, int32_t streamId);

// Unload audio stream and free memory
RAUDIO2_API void RAUDIO2_CALL RAudio2_UnloadAudioStream(RAUDIO2_HANDLE handle, int32_t streamId);

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: dequeue -> update -> queue
// NOTE 2: To dequeue a buffer it needs to be processed: IsAudioStreamProcessed()
RAUDIO2_API void RAUDIO2_CALL RAudio2_UpdateAudioStream(RAUDIO2_HANDLE handle, int32_t streamId, const void* dataIn, int64_t samplesCount);

// Check if any audio stream buffers requires refill
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsAudioStreamProcessed(RAUDIO2_HANDLE handle, int32_t streamId);

// Play audio stream
RAUDIO2_API void RAUDIO2_CALL RAudio2_PlayAudioStream(RAUDIO2_HANDLE handle, int32_t streamId);

// Pause audio stream
RAUDIO2_API void RAUDIO2_CALL RAudio2_PauseAudioStream(RAUDIO2_HANDLE handle, int32_t streamId);

// Resume audio stream
RAUDIO2_API void RAUDIO2_CALL RAudio2_ResumeAudioStream(RAUDIO2_HANDLE handle, int32_t streamId);

// Check if audio stream is playing
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsAudioStreamPlaying(RAUDIO2_HANDLE handle, int32_t streamId);

// Check if audio stream is stopped
RAUDIO2_API bool RAUDIO2_CALL RAudio2_IsAudioStreamStopped(RAUDIO2_HANDLE handle, int32_t streamId);

// Stop audio stream
RAUDIO2_API void RAUDIO2_CALL RAudio2_StopAudioStream(RAUDIO2_HANDLE handle, int32_t streamId);

// Set volume for audio stream (1.0 is max level)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetAudioStreamVolume(RAUDIO2_HANDLE handle, int32_t streamId, float volume);

// Set pitch for audio stream (1.0 is base level)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetAudioStreamPitch(RAUDIO2_HANDLE handle, int32_t streamId, float pitch);

// Set pan for audio stream  (0.0 to 1.0, 0.5=center)
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetAudioStreamPan(RAUDIO2_HANDLE handle, int32_t streamId, float pan);

// Default size for new audio streams
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetAudioStreamDefaultBufferSize(RAUDIO2_HANDLE handle, int32_t size);

// Audio thread callback to request new data
RAUDIO2_API void RAUDIO2_CALL RAudio2_SetAudioStreamCallback(RAUDIO2_HANDLE handle, int32_t streamId, AudioCallback callback);

// Attach audio stream processor to stream
// Contrary to buffers, the order of processors is important
// The new processor must be added at the end. As there aren't supposed to be a lot of processors attached to
// a given stream, we iterate through the list to find the end. That way we don't need a pointer to the last element.
RAUDIO2_API void RAUDIO2_CALL RAudio2_AttachAudioStreamProcessor(RAUDIO2_HANDLE handle, int32_t streamId, AudioCallback processor);

// Detach audio stream processor from stream
RAUDIO2_API void RAUDIO2_CALL RAudio2_DetachAudioStreamProcessor(RAUDIO2_HANDLE handle, int32_t streamId, AudioCallback processor);

// Attach audio stream processor to the entire audio pipeline
// Order of processors is important
// Works the same way as {Attach,Detach}AudioStreamProcessor() functions, except
// these two work on the already mixed output just before sending it to the sound hardware
RAUDIO2_API void RAUDIO2_CALL RAudio2_AttachAudioMixedProcessor(RAUDIO2_HANDLE handle, AudioCallback processor);

// Detach audio stream processor from the entire audio pipeline
RAUDIO2_API void RAUDIO2_CALL RAudio2_DetachAudioMixedProcessor(RAUDIO2_HANDLE handle, AudioCallback processor);

#ifdef __cplusplus
}
#endif

#endif // RAUDIO2_H
