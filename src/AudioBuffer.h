#pragma once

#include "AudioProcessor.h"
#include "miniaudio.h"

// NOTE: Different logic is used when feeding data to the playback device
// depending on whether data is streamed (Music vs Sound)
enum class AudioBufferUsage : int32_t
{
    Static,
    Stream
};

struct AudioData;

// Audio buffer struct
class AudioBuffer
{
public:
    ma_data_converter converter; // Audio data converter

    AudioCallback callback;    // Audio buffer callback for buffer filling on audio threads
    AudioProcessor* processor; // Audio processor

    float volume; // Audio buffer volume
    float pitch;  // Audio buffer pitch
    float pan;    // Audio buffer pan (0.0f to 1.0f)

    bool playing;           // Audio buffer state: AUDIO_PLAYING
    bool paused;            // Audio buffer state: AUDIO_PAUSED
    bool looping;           // Audio buffer looping, default to true for AudioStreams
    AudioBufferUsage usage; // Audio buffer usage mode: STATIC or STREAM

    bool isSubBufferProcessed[2]; // SubBuffer processed (virtual double buffer)
    int64_t sizeInFrames;         // Total buffer size in frames
    int64_t frameCursorPos;       // Frame cursor position
    int64_t framesProcessed;      // Total frames processed in this buffer (required for play timing)

    unsigned char* data; // Data buffer, on music stream keeps filling

    AudioBuffer* next; // Next audio buffer on the list
    AudioBuffer* prev; // Previous audio buffer on the list

    static AudioBuffer* Load(AudioData& audioData, ma_format format,
        ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, AudioBufferUsage usage);
    static void Unload(AudioData& audioData, AudioBuffer* buffer);

    bool IsPlaying();
    bool IsStopped();
    void Play();
    void Stop();
    void Pause();
    void Resume();
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetPan(float pan);

    static void TrackAudioBuffer(AudioData& audioData, AudioBuffer* buffer);
    static void UntrackAudioBuffer(AudioData& audioData, AudioBuffer* buffer);
};
