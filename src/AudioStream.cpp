#include "AudioStream.h"
#include "AudioDevice.h"
#include <cstring>
#include <miniaudio.h>
#include "raudio2/raudio2.hpp"
#include "SampleFormat.h"

AudioStream::AudioStream()
{
    static int32_t streamIDCounter = 1;
    ID = streamIDCounter++;
}

std::shared_ptr<AudioStream> AudioStream::Load(AudioDevice& audioDevice, RAudio2_SampleFormat sampleFormat, int32_t sampleRate, int32_t channels)
{
    auto stream = std::make_shared<AudioStream>();

    if (sampleFormat <= RAUDIO2_SAMPLE_FORMAT_UNKNOWN || sampleFormat >= RAUDIO2_SAMPLE_FORMAT_COUNT)
        sampleFormat = audioDevice.GetFormat();
    if (sampleRate == 0)
        sampleRate = audioDevice.GetSampleRate();
    if (channels == 0)
        channels = audioDevice.GetChannels();

    switch (sampleFormat)
    {
    case RAUDIO2_SAMPLE_FORMAT_U8:
        stream->sampleSize = 8;
        break;
    case RAUDIO2_SAMPLE_FORMAT_S16:
        stream->sampleSize = 16;
        break;
    default:
        stream->sampleSize = 32;
        break;
    }

    stream->sampleRate = sampleRate;
    stream->channels = channels;

    auto formatIn = GetMiniAudioFormat(sampleFormat);

    // The size of a streaming buffer must be at least double the size of a period
    unsigned int periodSize = audioDevice.GetAudioData().system.device.playback.internalPeriodSizeInFrames;

    // If the buffer is not set, compute one that would give us a buffer good enough for a decent frame rate
    unsigned int subBufferSize = (audioDevice.GetDefaultBufferSize() == 0) ? audioDevice.GetSampleRate() / 30 : audioDevice.GetDefaultBufferSize();

    if (subBufferSize < periodSize)
        subBufferSize = periodSize;

    // Create a double audio buffer of defined size
    stream->buffer = AudioBuffer::Load(audioDevice.GetAudioData(), formatIn, (ma_uint32)channels, (ma_uint32)sampleRate, subBufferSize * 2, AudioBufferUsage::Stream);

    if (stream->buffer != nullptr)
    {
        stream->buffer->looping = true; // Always loop for streaming buffers
        RAUDIO2_TRACELOG(LOG_INFO, "STREAM: Initialized successfully (%i Hz, %i bit, %s)",
            stream->sampleRate, stream->sampleSize, (stream->channels == 1) ? "Mono" : "Stereo");

        audioDevice.AddAudioStream(stream);
    }
    else
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "STREAM: Failed to load audio buffer, stream could not be created");

    return stream;
}

void AudioStream::Unload(AudioDevice& audioDevice)
{
    AudioBuffer::Unload(audioDevice.GetAudioData(), buffer);

    RAUDIO2_TRACELOG(LOG_INFO, "STREAM: Unloaded audio stream data from RAM");
}

bool AudioStream::IsReady()
{
    return ((buffer != nullptr) && // Validate stream buffer
            (sampleRate > 0) &&    // Validate sample rate is supported
            (sampleSize > 0) &&    // Validate sample size is supported
            (channels > 0));       // Validate number of channels supported
}

void AudioStream::Update(const void* dataIn, int64_t frameCount)
{
    if (buffer != nullptr)
    {
        if (buffer->isSubBufferProcessed[0] || buffer->isSubBufferProcessed[1])
        {
            ma_uint32 subBufferToUpdate = 0;

            if (buffer->isSubBufferProcessed[0] && buffer->isSubBufferProcessed[1])
            {
                // Both buffers are available for updating.
                // Update the first one and make sure the cursor is moved back to the front.
                subBufferToUpdate = 0;
                buffer->frameCursorPos = 0;
            }
            else
            {
                // Just update whichever sub-buffer is processed.
                subBufferToUpdate = (buffer->isSubBufferProcessed[0]) ? 0 : 1;
            }

            ma_uint32 subBufferSizeInFrames = buffer->sizeInFrames / 2;
            unsigned char* subBuffer = buffer->data + ((subBufferSizeInFrames * channels * (sampleSize / 8)) * subBufferToUpdate);

            // Total frames processed in buffer is always the complete size, filled with 0 if required
            buffer->framesProcessed += subBufferSizeInFrames;

            // Does this API expect a whole buffer to be updated in one go?
            // Assuming so, but if not will need to change this logic.
            if (subBufferSizeInFrames >= (ma_uint32)frameCount)
            {
                ma_uint32 framesToWrite = (ma_uint32)frameCount;

                ma_uint32 bytesToWrite = framesToWrite * channels * (sampleSize / 8);
                memcpy(subBuffer, dataIn, bytesToWrite);

                // Any leftover frames should be filled with zeros.
                ma_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;

                if (leftoverFrameCount > 0)
                    memset(subBuffer + bytesToWrite, 0, leftoverFrameCount * channels * (sampleSize / 8));

                buffer->isSubBufferProcessed[subBufferToUpdate] = false;
            }
            else
                RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "STREAM: Attempting to write too many frames to buffer");
        }
        else
            RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "STREAM: Buffer not available for updating");
    }
}

bool AudioStream::IsProcessed()
{
    if (buffer == nullptr)
        return false;

    return (buffer->isSubBufferProcessed[0] || buffer->isSubBufferProcessed[1]);
}

void AudioStream::Play()
{
    buffer->Play();
}

void AudioStream::Pause()
{
    buffer->Pause();
}

void AudioStream::Resume()
{
    buffer->Resume();
}

bool AudioStream::IsPlaying()
{
    return buffer->IsPlaying();
}

bool AudioStream::IsStopped()
{
    return buffer->IsStopped();
}

void AudioStream::Stop()
{
    buffer->Stop();
}

void AudioStream::SetVolume(float volume)
{
    buffer->SetVolume(volume);
}

void AudioStream::SetPitch(float pitch)
{
    buffer->SetPitch(pitch);
}

void AudioStream::SetPan(float pan)
{
    buffer->SetPan(pan);
}

void AudioStream::SetCallback(AudioCallback callback)
{
    if (buffer != nullptr)
        buffer->callback = callback;
}
