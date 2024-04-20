#include "AudioDevice.h"
#include <cstring>
#include "raudio2/raudio2_common.hpp"
#include "SampleFormat.h"
#include "Utils.h"

#if defined(RAUDIO2_ARCHIVE_GZIP)
#include <raudio2_gzip.h>
#endif

#if defined(RAUDIO2_ARCHIVE_LIBARCHIVE)
#include <raudio2_libarchive.h>
#endif

#if defined(RAUDIO2_INPUT_DRFLAC)
#include <raudio2_drflac.h>
#endif

#if defined(RAUDIO2_INPUT_DRMP3)
#include <raudio2_drmp3.h>
#endif

#if defined(RAUDIO2_INPUT_FLAC)
#include <raudio2_flac.h>
#endif

#if defined(RAUDIO2_INPUT_GME)
#include <raudio2_gme.h>
#endif

#if defined(RAUDIO2_INPUT_MODPLUG)
#include <raudio2_modplug.h>
#endif

#if defined(RAUDIO2_INPUT_MPG123)
#include <raudio2_mpg123.h>
#endif

#if defined(RAUDIO2_INPUT_OPENMPT)
#include <raudio2_openmpt.h>
#endif

#if defined(RAUDIO2_INPUT_OPUS)
#include <raudio2_opus.h>
#endif

#if defined(RAUDIO2_INPUT_QOA)
#include <raudio2_qoa.h>
#endif

#if defined(RAUDIO2_INPUT_SNDFILE)
#include <raudio2_sndfile.h>
#endif

#if defined(RAUDIO2_INPUT_STBVORBIS)
#include <raudio2_stbvorbis.h>
#endif

#if defined(RAUDIO2_INPUT_VORBIS)
#include <raudio2_vorbis.h>
#endif

#if defined(RAUDIO2_INPUT_WAV)
#include <raudio2_wav.h>
#endif

static void OnLog(void* pUserData, ma_uint32 level, const char* pMessage);
static void OnSendAudioDataToDevice(ma_device* pDevice, void* pFramesOut, const void* pFramesInput, ma_uint32 frameCount);
static void MixAudioFrames(AudioDevice& audioDevice, float* framesOut, const float* framesIn, ma_uint32 frameCount, AudioBuffer* buffer);

// Log callback function
static void OnLog(void* pUserData, ma_uint32 level, const char* pMessage)
{
    RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "miniaudio: %s", pMessage); // All log messages from miniaudio are errors
}

void AudioDevice::UpdateThreadFunction()
{
    while (audioData.system.isReady)
    {
        ma_mutex_lock(&audioData.system.lock);
        for (const auto& music : musics)
        {
            music.second->Update(audioData);
        }
        ma_mutex_unlock(&audioData.system.lock);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

AudioDevice::~AudioDevice()
{
    if (audioData.system.isReady)
        Uninit();
}

void AudioDevice::Init(int32_t flags)
{
    Init(RAUDIO2_AUDIO_DEVICE_FORMAT, RAUDIO2_AUDIO_DEVICE_SAMPLE_RATE, RAUDIO2_AUDIO_DEVICE_CHANNELS, flags);
}

void AudioDevice::Init(RAudio2_SampleFormat sampleFormat, int32_t sampleRate, int32_t channels, int32_t flags)
{
    // NOTE: Music buffer size is defined by number of samples, independent of sample size and channels number
    // After some math, considering a sampleRate of 48000, a buffer refill rate of 1/60 seconds and a
    // standard double-buffering system, a 4096 samples buffer has been chosen, it should be enough
    // In case of music-stalls, just increase this number
    audioData.buffer.defaultSize = 0;

#if defined(RAUDIO2_ARCHIVE_GZIP)
    RegisterArchivePlugin(GZIP_MakeArchivePlugin, true);
#endif
#if defined(RAUDIO2_ARCHIVE_LIBARCHIVE)
    RegisterArchivePlugin(LIBARCHIVE_MakeArchivePlugin, true);
#endif

#if defined(RAUDIO2_INPUT_WAV)
    RegisterInputPlugin(WAV_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_MPG123)
    RegisterInputPlugin(MPG123_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_DRMP3)
    RegisterInputPlugin(DRMP3_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_VORBIS)
    RegisterInputPlugin(VORBIS_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_STBVORBIS)
    RegisterInputPlugin(STBVORBIS_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_OPUS)
    RegisterInputPlugin(OPUS_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_FLAC)
    RegisterInputPlugin(FLAC_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_DRFLAC)
    RegisterInputPlugin(DRFLAC_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_OPENMPT)
    RegisterInputPlugin(OPENMPT_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_MODPLUG)
    RegisterInputPlugin(MODPLUG_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_GME)
    RegisterInputPlugin(GME_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_QOA)
    RegisterInputPlugin(QOA_MakeInputPlugin, true);
#endif
#if defined(RAUDIO2_INPUT_SNDFILE)
    RegisterInputPlugin(SNDFILE_MakeInputPlugin, true);
#endif

    // Init audio context
    ma_context_config ctxConfig = ma_context_config_init();
    ma_log_callback_init(OnLog, nullptr);

    ma_result result = ma_context_init(nullptr, 0, &ctxConfig, &audioData.system.context);
    if (result != MA_SUCCESS)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Failed to initialize context");
        return;
    }

    // Init audio device
    // NOTE: Using the default device. Format is floating point because it simplifies mixing.
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = nullptr; // nullptr for the default playback audioData.system.device.
    config.playback.format = GetMiniAudioFormat(sampleFormat);
    config.playback.channels = channels;
    config.capture.pDeviceID = nullptr; // nullptr for the default capture audioData.system.device.
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = sampleRate;
    config.dataCallback = OnSendAudioDataToDevice;
    config.pUserData = this;

    result = ma_device_init(&audioData.system.context, &config, &audioData.system.device);
    if (result != MA_SUCCESS)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Failed to initialize playback device");
        ma_context_uninit(&audioData.system.context);
        return;
    }

    // Mixing happens on a separate thread which means we need to synchronize. I'm using a mutex here to make things simple, but may
    // want to look at something a bit smarter later on to keep everything real-time, if that's necessary.
    if (ma_mutex_init(&audioData.system.lock) != MA_SUCCESS)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Failed to create mutex for mixing");
        ma_device_uninit(&audioData.system.device);
        ma_context_uninit(&audioData.system.context);
        return;
    }

    // Keep the device running the whole time. May want to consider doing something a bit smarter and only have the device running
    // while there's at least one sound being played.
    result = ma_device_start(&audioData.system.device);
    if (result != MA_SUCCESS)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Failed to start playback device");
        ma_device_uninit(&audioData.system.device);
        ma_context_uninit(&audioData.system.context);
        return;
    }

    RAUDIO2_TRACELOG(LOG_INFO, "AUDIO: Device initialized successfully");
    RAUDIO2_TRACELOG(LOG_INFO, "    > Backend:       miniaudio / %s", ma_get_backend_name(audioData.system.context.backend));
    RAUDIO2_TRACELOG(LOG_INFO, "    > Format:        %s -> %s", ma_get_format_name(audioData.system.device.playback.format), ma_get_format_name(audioData.system.device.playback.internalFormat));
    RAUDIO2_TRACELOG(LOG_INFO, "    > Channels:      %u -> %u", audioData.system.device.playback.channels, audioData.system.device.playback.internalChannels);
    RAUDIO2_TRACELOG(LOG_INFO, "    > Sample rate:   %u -> %u", audioData.system.device.sampleRate, audioData.system.device.playback.internalSampleRate);
    RAUDIO2_TRACELOG(LOG_INFO, "    > Periods size:  %u", audioData.system.device.playback.internalPeriodSizeInFrames * audioData.system.device.playback.internalPeriods);

    audioData.system.isReady = true;

    if (flags & RAUDIO2_FLAG_AUTOUPDATE)
    {
        updateThread = std::jthread(&AudioDevice::UpdateThreadFunction, this);
    }
}

void AudioDevice::Uninit()
{
    if (!audioData.system.isReady)
    {
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "AUDIO: Device could not be closed, not currently initialized");
        return;
    }

    audioData.system.isReady = false;

    if (updateThread.joinable())
        updateThread.join();

    for (auto& plugin : inputPlugins)
    {
        if (plugin->uninit)
            plugin->uninit();
    }

    for (auto& plugin : archivePlugins)
    {
        if (plugin->uninit)
            plugin->uninit();
    }

    ma_mutex_uninit(&audioData.system.lock);
    ma_device_uninit(&audioData.system.device);
    ma_context_uninit(&audioData.system.context);

    RAUDIO2_FREE(audioData.system.pcmBuffer);
    audioData.system.pcmBuffer = nullptr;
    audioData.system.pcmBufferSize = 0;

    musics.clear();
    streams.clear();
    inputPlugins.clear();
    archivePlugins.clear();

    RAUDIO2_TRACELOG(LOG_INFO, "AUDIO: Device closed successfully");
}

bool AudioDevice::IsReady() const
{
    return audioData.system.isReady;
}

bool AudioDevice::RegisterPlugin(const char* filePath, RAudio2_PluginType pluginType, bool append)
{
    auto handle = LoadExternalLibrary(filePath);
    if (!handle)
        return false;

    bool success = false;

    if (pluginType == RAUDIO2_PLUGIN_ARCHIVE || pluginType == RAUDIO2_PLUGIN_ANY)
    {
        auto pluginFunc = (RAudio2_GetArchivePluginFunc)GetFunctionAddress(handle, "RAudio2_GetArchivePlugin");
        if (pluginFunc)
            success |= RegisterArchivePlugin(pluginFunc, append);
    }

    if (pluginType == RAUDIO2_PLUGIN_INPUT || pluginType == RAUDIO2_PLUGIN_ANY)
    {
        auto pluginFunc = (RAudio2_GetInputPluginFunc)GetFunctionAddress(handle, "RAudio2_GetInputPlugin");
        if (pluginFunc)
            success |= RegisterInputPlugin(pluginFunc, append);
    }

    return success;
}

bool AudioDevice::RegisterArchivePlugin(bool (*getArchivePlugin)(RAudio2_ArchivePlugin*), bool append)
{
    RAudio2_ArchivePlugin plugin;
    if (getArchivePlugin(&plugin))
        return RegisterArchivePlugin(plugin, append);

    return false;
}

bool AudioDevice::RegisterArchivePlugin(const RAudio2_ArchivePlugin& plugin, bool append)
{
    if (plugin.flags ||
        !plugin.archiveOpen ||
        !plugin.archiveClose ||
        !plugin.fileOpen ||
        !plugin.fileRead ||
        !plugin.fileSeek ||
        !plugin.fileTell ||
        !plugin.fileGetSize ||
        !plugin.fileClose ||
        !plugin.getValue)
        return false;

    if (plugin.init)
        plugin.init();

    ra::ArchivePlugin raPlugin(plugin);
    auto name = raPlugin.getStringView("plugin_name");
    if (name.empty())
    {
        if (plugin.uninit)
            plugin.uninit();
        return false;
    }

    if (append)
        archivePlugins.push_back(std::make_unique<RAudio2_ArchivePlugin>(plugin));
    else
        archivePlugins.insert(archivePlugins.begin(), std::make_unique<RAudio2_ArchivePlugin>(plugin));

    archivePluginNames.insert(archivePluginNames.end() - 1, name.data());

    return true;
}

bool AudioDevice::RegisterInputPlugin(bool (*getInputPlugin)(RAudio2_InputPlugin*), bool append)
{
    RAudio2_InputPlugin plugin;
    if (getInputPlugin(&plugin))
        return RegisterInputPlugin(plugin, append);

    return false;
}

bool AudioDevice::RegisterInputPlugin(const RAudio2_InputPlugin& plugin, bool append)
{
    if (plugin.flags ||
        !plugin.open ||
        !plugin.read ||
        !plugin.seek ||
        !plugin.close ||
        !plugin.getValue)
        return false;

    if (plugin.init)
        plugin.init();

    ra::InputPlugin raPlugin(plugin);
    auto name = raPlugin.getStringView("plugin_name");
    if (name.empty())
    {
        if (plugin.uninit)
            plugin.uninit();
        return false;
    }

    if (append)
        inputPlugins.push_back(std::make_unique<RAudio2_InputPlugin>(plugin));
    else
        inputPlugins.insert(inputPlugins.begin(), std::make_unique<RAudio2_InputPlugin>(plugin));

    inputPluginNames.insert(inputPluginNames.end() - 1, name.data());

    return true;
}

int32_t AudioDevice::AddAudioStream(const std::shared_ptr<AudioStream>& stream)
{
    auto id = stream->GetID();
    streams.emplace(id, stream);
    return id;
}

int32_t AudioDevice::AddMusic(std::unique_ptr<Music>&& music)
{
    auto id = music->GetID();
    musics.emplace(id, std::move(music));
    return id;
}

AudioStream* AudioDevice::GetAudioStream(int32_t streamId) const
{
    auto it = streams.find(streamId);
    if (it != streams.end())
        return it->second.get();
    return nullptr;
}

Music* AudioDevice::GetMusic(int32_t musicId) const
{
    auto it = musics.find(musicId);
    if (it != musics.end())
        return it->second.get();
    return nullptr;
}

bool AudioDevice::DeleteAudioStream(int32_t streamId)
{
    return streams.erase(streamId) > 0;
}

bool AudioDevice::DeleteMusic(int32_t musicId)
{
    return musics.erase(musicId) > 0;
}

RAudio2_SampleFormat AudioDevice::GetFormat()
{
    return (RAudio2_SampleFormat)audioData.system.device.playback.format;
}

int32_t AudioDevice::GetSampleRate()
{
    return (int32_t)audioData.system.device.sampleRate;
}

int32_t AudioDevice::GetChannels()
{
    return (int32_t)audioData.system.device.playback.channels;
}

int32_t AudioDevice::GetDefaultBufferSize()
{
    return audioData.buffer.defaultSize;
}

float AudioDevice::GetMasterVolume()
{
    float volume = 0.0f;
    ma_device_get_master_volume(&audioData.system.device, &volume);
    return volume;
}

void AudioDevice::SetMasterVolume(float volume)
{
    ma_device_set_master_volume(&audioData.system.device, volume);
}

// Reads audio data from an AudioBuffer object in internal format.
static ma_uint32 ReadAudioBufferFramesInInternalFormat(AudioBuffer* audioBuffer, void* framesOut, ma_uint32 frameCount)
{
    // Using audio buffer callback
    if (audioBuffer->callback)
    {
        audioBuffer->callback(framesOut, frameCount);
        audioBuffer->framesProcessed += frameCount;

        return frameCount;
    }

    ma_uint32 subBufferSizeInFrames = (audioBuffer->sizeInFrames > 1) ? audioBuffer->sizeInFrames / 2 : audioBuffer->sizeInFrames;
    ma_uint32 currentSubBufferIndex = audioBuffer->frameCursorPos / subBufferSizeInFrames;

    if (currentSubBufferIndex > 1)
        return 0;

    // Another thread can update the processed state of buffers, so
    // we just take a copy here to try and avoid potential synchronization problems
    bool isSubBufferProcessed[2] = { 0 };
    isSubBufferProcessed[0] = audioBuffer->isSubBufferProcessed[0];
    isSubBufferProcessed[1] = audioBuffer->isSubBufferProcessed[1];

    ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(audioBuffer->converter.formatIn, audioBuffer->converter.channelsIn);

    // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0
    ma_uint32 framesRead = 0;
    while (1)
    {
        // We break from this loop differently depending on the buffer's usage
        //  - For static buffers, we simply fill as much data as we can
        //  - For streaming buffers we only fill half of the buffer that are processed
        //    Unprocessed halves must keep their audio data in-tact
        if (audioBuffer->usage == AudioBufferUsage::Static)
        {
            if (framesRead >= frameCount)
                break;
        }
        else
        {
            if (isSubBufferProcessed[currentSubBufferIndex])
                break;
        }

        ma_uint32 totalFramesRemaining = (frameCount - framesRead);
        if (totalFramesRemaining == 0)
            break;

        ma_uint32 framesRemainingInOutputBuffer;
        if (audioBuffer->usage == AudioBufferUsage::Static)
        {
            framesRemainingInOutputBuffer = audioBuffer->sizeInFrames - audioBuffer->frameCursorPos;
        }
        else
        {
            ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames * currentSubBufferIndex;
            framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos - firstFrameIndexOfThisSubBuffer);
        }

        ma_uint32 framesToRead = totalFramesRemaining;
        if (framesToRead > framesRemainingInOutputBuffer)
            framesToRead = framesRemainingInOutputBuffer;

        memcpy((unsigned char*)framesOut + (framesRead * frameSizeInBytes), audioBuffer->data + (audioBuffer->frameCursorPos * frameSizeInBytes), framesToRead * frameSizeInBytes);
        audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead) % audioBuffer->sizeInFrames;
        framesRead += framesToRead;

        // If we've read to the end of the buffer, mark it as processed
        if (framesToRead == framesRemainingInOutputBuffer)
        {
            audioBuffer->isSubBufferProcessed[currentSubBufferIndex] = true;
            isSubBufferProcessed[currentSubBufferIndex] = true;

            currentSubBufferIndex = (currentSubBufferIndex + 1) % 2;

            // We need to break from this loop if we're not looping
            if (!audioBuffer->looping)
            {
                audioBuffer->Stop();
                break;
            }
        }
    }

    // Zero-fill excess
    ma_uint32 totalFramesRemaining = (frameCount - framesRead);
    if (totalFramesRemaining > 0)
    {
        memset((unsigned char*)framesOut + (framesRead * frameSizeInBytes), 0, totalFramesRemaining * frameSizeInBytes);

        // For static buffers we can fill the remaining frames with silence for safety, but we don't want
        // to report those frames as "read". The reason for this is that the caller uses the return value
        // to know whether a non-looping sound has finished playback.
        if (audioBuffer->usage != AudioBufferUsage::Static)
            framesRead += totalFramesRemaining;
    }

    return framesRead;
}

// Reads audio data from an AudioBuffer object in device format. Returned data will be in a format appropriate for mixing.
static ma_uint32 ReadAudioBufferFramesInMixingFormat(AudioBuffer* audioBuffer, float* framesOut, ma_uint32 frameCount)
{
    // What's going on here is that we're continuously converting data from the AudioBuffer's internal format to the mixing format, which
    // should be defined by the output format of the data converter. We do this until frameCount frames have been output. The important
    // detail to remember here is that we never, ever attempt to read more input data than is required for the specified number of output
    // frames. This can be achieved with ma_data_converter_get_required_input_frame_count().
    ma_uint8 inputBuffer[4096] = { 0 };
    ma_uint32 inputBufferFrameCap = sizeof(inputBuffer) / ma_get_bytes_per_frame(audioBuffer->converter.formatIn, audioBuffer->converter.channelsIn);

    ma_uint32 totalOutputFramesProcessed = 0;
    while (totalOutputFramesProcessed < frameCount)
    {
        ma_uint64 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;
        ma_uint64 inputFramesToProcessThisIteration = 0;

        (void)ma_data_converter_get_required_input_frame_count(&audioBuffer->converter, outputFramesToProcessThisIteration, &inputFramesToProcessThisIteration);
        if (inputFramesToProcessThisIteration > inputBufferFrameCap)
        {
            inputFramesToProcessThisIteration = inputBufferFrameCap;
        }

        float* runningFramesOut = framesOut + (totalOutputFramesProcessed * audioBuffer->converter.channelsOut);

        /* At this point we can convert the data to our mixing format. */
        ma_uint64 inputFramesProcessedThisIteration = ReadAudioBufferFramesInInternalFormat(audioBuffer, inputBuffer, (ma_uint32)inputFramesToProcessThisIteration); /* Safe cast. */
        ma_uint64 outputFramesProcessedThisIteration = outputFramesToProcessThisIteration;
        ma_data_converter_process_pcm_frames(&audioBuffer->converter, inputBuffer, &inputFramesProcessedThisIteration, runningFramesOut, &outputFramesProcessedThisIteration);

        totalOutputFramesProcessed += (ma_uint32)outputFramesProcessedThisIteration; /* Safe cast. */

        if (inputFramesProcessedThisIteration < inputFramesToProcessThisIteration)
        {
            break; /* Ran out of input data. */
        }

        /* This should never be hit, but will add it here for safety. Ensures we get out of the loop when no input nor output frames are processed. */
        if (inputFramesProcessedThisIteration == 0 && outputFramesProcessedThisIteration == 0)
        {
            break;
        }
    }

    return totalOutputFramesProcessed;
}

// Sending audio data to device callback function
// This function will be called when miniaudio needs more data
// NOTE: All the mixing takes place here
static void OnSendAudioDataToDevice(ma_device* pDevice, void* pFramesOut, const void* pFramesInput, ma_uint32 frameCount)
{
    auto audioDevice = (AudioDevice*)pDevice->pUserData;

    // Mixing is basically just an accumulation, we need to initialize the output buffer to 0
    memset(pFramesOut, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));

    // Using a mutex here for thread-safety which makes things not real-time
    // This is unlikely to be necessary for this project, but may want to consider how you might want to avoid this
    ma_mutex_lock(&audioDevice->GetAudioData().system.lock);
    {
        for (AudioBuffer* audioBuffer = audioDevice->GetAudioData().buffer.first; audioBuffer != nullptr; audioBuffer = audioBuffer->next)
        {
            // Ignore stopped or paused sounds
            if (!audioBuffer->playing || audioBuffer->paused)
                continue;

            ma_uint32 framesRead = 0;

            while (1)
            {
                if (framesRead >= frameCount)
                    break;

                // Just read as much data as we can from the stream
                ma_uint32 framesToRead = (frameCount - framesRead);

                while (framesToRead > 0)
                {
                    float tempBuffer[1024] = { 0 }; // Frames for stereo

                    ma_uint32 framesToReadRightNow = framesToRead;
                    if (framesToReadRightNow > sizeof(tempBuffer) / sizeof(tempBuffer[0]) / RAUDIO2_AUDIO_DEVICE_CHANNELS)
                    {
                        framesToReadRightNow = sizeof(tempBuffer) / sizeof(tempBuffer[0]) / RAUDIO2_AUDIO_DEVICE_CHANNELS;
                    }

                    ma_uint32 framesJustRead = ReadAudioBufferFramesInMixingFormat(audioBuffer, tempBuffer, framesToReadRightNow);
                    if (framesJustRead > 0)
                    {
                        float* framesOut = (float*)pFramesOut + (framesRead * audioDevice->GetAudioData().system.device.playback.channels);
                        float* framesIn = tempBuffer;

                        // Apply processors chain if defined
                        AudioProcessor* processor = audioBuffer->processor;
                        while (processor)
                        {
                            processor->process(framesIn, framesJustRead);
                            processor = processor->next;
                        }

                        MixAudioFrames(*audioDevice, framesOut, framesIn, framesJustRead, audioBuffer);

                        framesToRead -= framesJustRead;
                        framesRead += framesJustRead;
                    }

                    if (!audioBuffer->playing)
                    {
                        framesRead = frameCount;
                        break;
                    }

                    // If we weren't able to read all the frames we requested, break
                    if (framesJustRead < framesToReadRightNow)
                    {
                        if (!audioBuffer->looping)
                        {
                            audioBuffer->Stop();
                            break;
                        }
                        else
                        {
                            // Should never get here, but just for safety,
                            // move the cursor position back to the start and continue the loop
                            audioBuffer->frameCursorPos = 0;
                            continue;
                        }
                    }
                }

                // If for some reason we weren't able to read every frame we'll need to break from the loop
                // Not doing this could theoretically put us into an infinite loop
                if (framesToRead > 0)
                    break;
            }
        }
    }

    AudioProcessor* processor = audioDevice->GetAudioData().mixedProcessor;
    while (processor)
    {
        processor->process(pFramesOut, frameCount);
        processor = processor->next;
    }

    ma_mutex_unlock(&audioDevice->GetAudioData().system.lock);
}

// Main mixing function, pretty simple in this project, just an accumulation
// NOTE: framesOut is both an input and an output, it is initially filled with zeros outside of this function
static void MixAudioFrames(AudioDevice& audioDevice, float* framesOut, const float* framesIn, ma_uint32 frameCount, AudioBuffer* buffer)
{
    const float localVolume = buffer->volume;
    const ma_uint32 channels = audioDevice.GetAudioData().system.device.playback.channels;

    if (channels == 2) // We consider panning
    {
        const float left = buffer->pan;
        const float right = 1.0f - left;

        // Fast sine approximation in [0..1] for pan law: y = 0.5f*x*(3 - x*x);
        const float levels[2] = { localVolume * 0.5f * left * (3.0f - left * left), localVolume * 0.5f * right * (3.0f - right * right) };

        float* frameOut = framesOut;
        const float* frameIn = framesIn;

        for (ma_uint32 frame = 0; frame < frameCount; frame++)
        {
            frameOut[0] += (frameIn[0] * levels[0]);
            frameOut[1] += (frameIn[1] * levels[1]);

            frameOut += 2;
            frameIn += 2;
        }
    }
    else // We do not consider panning
    {
        for (ma_uint32 frame = 0; frame < frameCount; frame++)
        {
            for (ma_uint32 c = 0; c < channels; c++)
            {
                float* frameOut = framesOut + (frame * channels);
                const float* frameIn = framesIn + (frame * channels);

                // Output accumulates input multiplied by volume to provided output (usually 0)
                frameOut[c] += (frameIn[c] * localVolume);
            }
        }
    }
}

void AudioDevice::AttachAudioStreamProcessor(int32_t streamId, AudioCallback process)
{
    ma_mutex_lock(&audioData.system.lock);

    auto stream = GetAudioStream(streamId);
    if (!stream)
        return;

    auto processor = (AudioProcessor*)RAUDIO2_CALLOC(1, sizeof(AudioProcessor));
    if (!processor)
        return;

    processor->process = process;

    AudioProcessor* last = stream->buffer->processor;

    while (last && last->next)
    {
        last = last->next;
    }
    if (last)
    {
        processor->prev = last;
        last->next = processor;
    }
    else
        stream->buffer->processor = processor;

    ma_mutex_unlock(&audioData.system.lock);
}

void AudioDevice::DetachAudioStreamProcessor(int32_t streamId, AudioCallback process)
{
    ma_mutex_lock(&audioData.system.lock);

    auto stream = GetAudioStream(streamId);
    if (!stream)
        return;

    AudioProcessor* processor = stream->buffer->processor;

    while (processor)
    {
        AudioProcessor* next = processor->next;
        AudioProcessor* prev = processor->prev;

        if (processor->process == process)
        {
            if (stream->buffer->processor == processor)
                stream->buffer->processor = next;
            if (prev)
                prev->next = next;
            if (next)
                next->prev = prev;

            RAUDIO2_FREE(processor);
        }

        processor = next;
    }

    ma_mutex_unlock(&audioData.system.lock);
}

void AudioDevice::AttachAudioMixedProcessor(AudioCallback process)
{
    ma_mutex_lock(&audioData.system.lock);

    auto processor = (AudioProcessor*)RAUDIO2_CALLOC(1, sizeof(AudioProcessor));
    if (!processor)
        return;

    processor->process = process;

    AudioProcessor* last = audioData.mixedProcessor;

    while (last && last->next)
    {
        last = last->next;
    }
    if (last)
    {
        processor->prev = last;
        last->next = processor;
    }
    else
        audioData.mixedProcessor = processor;

    ma_mutex_unlock(&audioData.system.lock);
}

void AudioDevice::DetachAudioMixedProcessor(AudioCallback process)
{
    ma_mutex_lock(&audioData.system.lock);

    AudioProcessor* processor = audioData.mixedProcessor;

    while (processor)
    {
        AudioProcessor* next = processor->next;
        AudioProcessor* prev = processor->prev;

        if (processor->process == process)
        {
            if (audioData.mixedProcessor == processor)
                audioData.mixedProcessor = next;
            if (prev)
                prev->next = next;
            if (next)
                next->prev = prev;

            RAUDIO2_FREE(processor);
        }

        processor = next;
    }

    ma_mutex_unlock(&audioData.system.lock);
}

bool AudioDevice::GetValue(const char* key_, int32_t keyLength, RAudio2_Value* valueOut) const noexcept
{
    auto key = std::string_view(key_, keyLength);

    auto [firstKey, query] = ra::splitKey(key);

    auto keyHash = ra::str2int(firstKey.substr(0, 32));
    switch (keyHash)
    {
    case ra::str2int("archive"): {

        auto [pluginName, pluginQuery] = ra::splitKey(query);

        for (auto& plugin : archivePlugins)
        {
            ra::ArchivePlugin raPlugin(plugin.get());

            if (raPlugin.getStringView("plugin_name") == pluginName)
                return raPlugin.getValue(pluginQuery, valueOut);
        }
        return false;
    }
    case ra::str2int("archive_plugins"): {
        return ra::MakeArrayValue(archivePluginNames, *valueOut);
        return true;
    }
    case ra::str2int("input"): {

        auto [pluginName, pluginQuery] = ra::splitKey(query);

        for (auto& plugin : inputPlugins)
        {
            ra::InputPlugin raPlugin(plugin.get());

            if (raPlugin.getStringView("plugin_name") == pluginName)
                return raPlugin.getValue(pluginQuery, valueOut);
        }
        return false;
    }
    case ra::str2int("input_plugins"): {
        return ra::MakeArrayValue(inputPluginNames, *valueOut);
        return true;
    }
    default:
        break;
    }
    *valueOut = {};
    return false;
}
