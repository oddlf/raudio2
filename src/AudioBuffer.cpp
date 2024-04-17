#include "AudioBuffer.h"
#include "AudioData.h"
#include "raudio2/raudio2.hpp"
#include "SampleFormat.h"

// Initialize a new audio buffer (filled with silence)
AudioBuffer* AudioBuffer::Load(AudioData& audioData, ma_format format,
    ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, AudioBufferUsage usage)
{
    auto audioBuffer = (AudioBuffer*)RAUDIO2_CALLOC(1, sizeof(AudioBuffer));
    if (!audioBuffer)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Failed to allocate memory for buffer");
        return {};
    }

    if (sizeInFrames > 0)
        audioBuffer->data = (unsigned char*)RAUDIO2_CALLOC(sizeInFrames * channels * ma_get_bytes_per_sample(format), 1);

    // Audio data runs through a format converter
    ma_data_converter_config converterConfig = ma_data_converter_config_init(
        format,
        GetMiniAudioFormat(RAUDIO2_AUDIO_DEVICE_FORMAT),
        channels,
        RAUDIO2_AUDIO_DEVICE_CHANNELS,
        sampleRate,
        audioData.system.device.sampleRate);
    converterConfig.allowDynamicSampleRate = true;

    ma_result result = ma_data_converter_init(&converterConfig, nullptr, &audioBuffer->converter);

    if (result != MA_SUCCESS)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Failed to create data conversion pipeline");
        RAUDIO2_FREE(audioBuffer);
        return {};
    }

    // Init audio buffer values
    audioBuffer->volume = 1.0f;
    audioBuffer->pitch = 1.0f;
    audioBuffer->pan = 0.5f;

    audioBuffer->callback = nullptr;
    audioBuffer->processor = nullptr;

    audioBuffer->playing = false;
    audioBuffer->paused = false;
    audioBuffer->looping = false;

    audioBuffer->usage = usage;
    audioBuffer->frameCursorPos = 0;
    audioBuffer->sizeInFrames = sizeInFrames;

    // Buffers should be marked as processed by default so that a call to
    // UpdateAudioStream() immediately after initialization works correctly
    audioBuffer->isSubBufferProcessed[0] = true;
    audioBuffer->isSubBufferProcessed[1] = true;

    // Track audio buffer to linked list next position
    TrackAudioBuffer(audioData, audioBuffer);

    return audioBuffer;
}

// Delete an audio buffer
void AudioBuffer::Unload(AudioData& audioData, AudioBuffer* buffer)
{
    if (buffer != nullptr)
    {
        ma_data_converter_uninit(&buffer->converter, nullptr);
        UntrackAudioBuffer(audioData, buffer);
        RAUDIO2_FREE(buffer->data);
        RAUDIO2_FREE(buffer);
    }
}

// Check if an audio buffer is playing
bool AudioBuffer::IsPlaying()
{
    return playing && !paused;
}

// Check if an audio buffer is stopped
bool AudioBuffer::IsStopped()
{
    bool result = true;

    if (!IsPlaying())
        result = frameCursorPos == 0;

    return result;
}

// Play an audio buffer
// NOTE: Buffer is restarted to the start.
// Use PauseAudioBuffer() and ResumeAudioBuffer() if the playback position should be maintained.
void AudioBuffer::Play()
{
    playing = true;
    paused = false;
    frameCursorPos = 0;
}

// Stop an audio buffer
void AudioBuffer::Stop()
{
    if (IsPlaying())
    {
        playing = false;
        paused = false;
        frameCursorPos = 0;
        framesProcessed = 0;
        isSubBufferProcessed[0] = true;
        isSubBufferProcessed[1] = true;
    }
}

// Pause an audio buffer
void AudioBuffer::Pause()
{
    paused = true;
}

// Resume an audio buffer
void AudioBuffer::Resume()
{
    paused = false;
}

// Set volume for an audio buffer
void AudioBuffer::SetVolume(float volume_)
{
    volume = volume_;
}

// Set pitch for an audio buffer
void AudioBuffer::SetPitch(float pitch_)
{
    if (pitch_ > 0.0f)
    {
        // Pitching is just an adjustment of the sample rate.
        // Note that this changes the duration of the sound:
        //  - higher pitches will make the sound faster
        //  - lower pitches make it slower
        ma_uint32 outputSampleRate = (ma_uint32)((float)converter.sampleRateOut / pitch_);
        ma_data_converter_set_rate(&converter, converter.sampleRateIn, outputSampleRate);

        pitch = pitch_;
    }
}

// Set pan for an audio buffer
void AudioBuffer::SetPan(float pan_)
{
    if (pan_ < 0.0f)
        pan_ = 0.0f;
    else if (pan_ > 1.0f)
        pan_ = 1.0f;

    pan = pan_;
}

// Track audio buffer to linked list next position
void AudioBuffer::TrackAudioBuffer(AudioData& audioData, AudioBuffer* buffer)
{
    ma_mutex_lock(&audioData.system.lock);
    {
        if (audioData.buffer.first == nullptr)
            audioData.buffer.first = buffer;
        else
        {
            audioData.buffer.last->next = buffer;
            buffer->prev = audioData.buffer.last;
        }

        audioData.buffer.last = buffer;
    }
    ma_mutex_unlock(&audioData.system.lock);
}

// Untrack audio buffer from linked list
void AudioBuffer::UntrackAudioBuffer(AudioData& audioData, AudioBuffer* buffer)
{
    ma_mutex_lock(&audioData.system.lock);
    {
        if (buffer->prev == nullptr)
            audioData.buffer.first = buffer->next;
        else
            buffer->prev->next = buffer->next;

        if (buffer->next == nullptr)
            audioData.buffer.last = buffer->prev;
        else
            buffer->next->prev = buffer->prev;

        buffer->prev = nullptr;
        buffer->next = nullptr;
    }
    ma_mutex_unlock(&audioData.system.lock);
}
