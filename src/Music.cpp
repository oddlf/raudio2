#include "Music.h"
#include "ArchivePluginIO.h"
#include "AudioData.h"
#include "AudioDevice.h"
#include <cinttypes>
#include "FileIO.h"
#include "MemoryDataIO.h"
#include <string_view>
#include "Utils.h"

Music::Music()
{
    static int32_t musicIDCounter = 1;
    ID = musicIDCounter++;
}

int32_t Music::Load(AudioDevice& audioDevice, const char* fileName, bool streamFile, VirtualIOWrapper&& file)
{
    auto music = std::make_unique<Music>();
    bool musicLoaded = false;

    auto [filePath, subFilePath] = SplitStringIn2(fileName, '|');

    if (!file)
    {
        file.setFile(std::make_unique<FileIO>(filePath.c_str(), "rb"));

        for (const auto& plugintPtr : audioDevice.ArchivePlugins())
        {
            ra::ArchivePlugin plugin(*plugintPtr);
            RAudio2_Archive tempArchive;
            tempArchive.file = &file.GetVirtualIO();
            void* tempFileCtx{};

            file.seek(0, RAUDIO2_SEEK_SET);

            if (!plugin.archiveOpen(&tempArchive))
                continue;

            if (!plugin.fileOpen(&tempArchive, subFilePath.c_str(), &tempFileCtx))
                continue;

            music->archive = tempArchive;
            music->archiveFileCtx = tempFileCtx;
            music->archivePlugin = plugin;

            music->archiveFile = std::move(file);
            file = VirtualIOWrapper(std::make_unique<ArchivePluginIO>(music->archivePlugin, tempFileCtx));
            break;
        }

        if (!music->archivePlugin)
        {
            if (streamFile)
                file.seek(0, RAUDIO2_SEEK_SET);
            else
                file.setFile(std::make_unique<MemoryDataIO>(filePath.c_str()));
        }
    }

    if (!file)
        return false;

    if (!subFilePath.empty())
        filePath = std::move(subFilePath);

    music->file = std::move(file);
    music->waveInfo.file = &music->file.GetVirtualIO();

    for (const auto& plugintPtr : audioDevice.InputPlugins())
    {
        ra::InputPlugin plugin(*plugintPtr);

        for (auto extPtr = plugin.getExtensions(); *extPtr; extPtr++)
        {
            if (IsFileExtension(filePath.c_str(), *extPtr))
            {
                music->inputPlugin = plugin;
                break;
            }
        }

        if (music->inputPlugin)
            break;
    }

    int32_t musicID = {};

    if (music->inputPlugin)
    {
        musicLoaded = music->inputPlugin.open(&music->waveInfo);

        if (!musicLoaded)
        {
            music->inputPlugin.close(&music->waveInfo);
            music->waveInfo.file = nullptr;
            music->waveInfo.ctxData = nullptr;
            music->file.setFile(nullptr);
            RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "FILEIO: Music file could not be opened");
        }
        else
        {
            music->stream = AudioStream::Load(
                audioDevice,
                (RAudio2_SampleFormat)music->waveInfo.sampleFormat,
                music->waveInfo.sampleRate,
                music->waveInfo.channels);
            music->frameCount = music->waveInfo.frameCount;

            // Show some music stream info
            RAUDIO2_TRACELOG(LOG_INFO, "FILEIO: Music file loaded successfully");
            RAUDIO2_TRACELOG(LOG_INFO, "    > Sample rate:   %" PRIi32 " Hz", music->stream->GetSampleRate());
            RAUDIO2_TRACELOG(LOG_INFO, "    > Sample size:   %" PRIi32 " bits", music->stream->GetSampleSize());
            RAUDIO2_TRACELOG(LOG_INFO, "    > Channels:      %" PRIi32 " (%s)",
                music->stream->GetChannels(), (music->stream->GetChannels() == 1) ? "Mono" : (music->stream->GetChannels() == 2) ? "Stereo"
                                                                                                                                 : "Multi");
            RAUDIO2_TRACELOG(LOG_INFO, "    > Total frames:  %" PRIi64, music->frameCount);

            musicID = audioDevice.AddMusic(std::move(music));
        }
    }
    else
        RAUDIO2_TRACELOG(RAUDIO2_LOG_WARNING, "STREAM: File format not supported");

    return musicID;
}

int32_t Music::Load(AudioDevice& audioDevice, const char* fileName, bool streamFile)
{
    return Load(audioDevice, fileName, streamFile, {});
}

int32_t Music::LoadFromMemory(AudioDevice& audioDevice, const char* fileType, const unsigned char* dataIn, int64_t dataSize)
{
    return Load(audioDevice, fileType, false, VirtualIOWrapper(std::make_unique<MemoryIO>(dataIn, dataSize)));
}

bool Music::IsReady()
{
    return ((waveInfo.ctxData != nullptr) && // Validate context loaded
            (frameCount > 0) &&              // Validate audio frame count
            (stream->GetSampleRate() > 0) && // Validate sample rate is supported
            (stream->GetSampleSize() > 0) && // Validate sample size is supported
            (stream->GetChannels() > 0));    // Validate number of channels supported
}

void Music::Unload(AudioDevice& audioDevice)
{
    stream->Unload(audioDevice);
    inputPlugin.close(&waveInfo);
    if (archivePlugin)
    {
        archivePlugin.fileClose(archiveFileCtx);
        archivePlugin.archiveClose(&archive);
    }
    audioDevice.DeleteMusic(ID);
}

void Music::Play()
{
    if (stream->buffer != nullptr)
    {
        // For music streams, we need to make sure we maintain the frame cursor position
        // This is a hack for this section of code in UpdateMusicStream()
        // NOTE: In case window is minimized, music stream is stopped, just make sure to
        // play again on window restore: if (IsMusicStreamPlaying(music)) PlayMusicStream(music);
        ma_uint32 frameCursorPos = stream->buffer->frameCursorPos;
        stream->Play(); // WARNING: This resets the cursor position.
        stream->buffer->frameCursorPos = frameCursorPos;
    }
}

bool Music::IsPlaying()
{
    return stream->IsPlaying();
}

bool Music::IsStopped()
{
    return stream->IsStopped();
}

void Music::Update(AudioData& audioData)
{
    if (stream->buffer == nullptr)
        return;

    auto subBufferSizeInFrames = stream->buffer->sizeInFrames / 2;

    // On first call of this function we lazily pre-allocated a temp buffer to read audio files/memory data in
    auto frameSize = stream->GetChannels() * stream->GetSampleSize() / 8;
    auto pcmSize = (int32_t)subBufferSizeInFrames * frameSize;

    if (audioData.system.pcmBufferSize < pcmSize)
    {
        RAUDIO2_FREE(audioData.system.pcmBuffer);
        audioData.system.pcmBuffer = RAUDIO2_CALLOC(1, (size_t)pcmSize);
        audioData.system.pcmBufferSize = pcmSize;
    }

    // Check both sub-buffers to check if they require refilling
    for (int i = 0; i < 2; i++)
    {
        if ((stream->buffer != nullptr) && !stream->buffer->isSubBufferProcessed[i])
            continue; // No refilling required, move to next sub-buffer

        auto framesLeft = frameCount - stream->buffer->framesProcessed; // Frames left to be processed
        int64_t framesToStream = 0;                                     // Total frames to be streamed

        if ((framesLeft >= subBufferSizeInFrames) || looping)
            framesToStream = subBufferSizeInFrames;
        else
            framesToStream = framesLeft;

        auto frameCountStillNeeded = framesToStream;
        int64_t frameCountReadTotal = 0;

        while (true)
        {
            auto frameCountRead = (int64_t)inputPlugin.read(&waveInfo, (short*)((char*)audioData.system.pcmBuffer + frameCountReadTotal * frameSize), frameCountStillNeeded);
            if (frameCountRead <= 0)
                break;

            frameCountReadTotal += frameCountRead;
            frameCountStillNeeded -= frameCountRead;
            if (frameCountStillNeeded == 0)
                break;
            else
                inputPlugin.seek(&waveInfo, 0);
        }

        stream->Update(audioData.system.pcmBuffer, framesToStream);

        stream->buffer->framesProcessed = stream->buffer->framesProcessed % frameCount;

        if (framesLeft <= subBufferSizeInFrames)
        {
            if (!looping)
            {
                // Streaming is ending, we filled latest frames from input
                Stop();
                return;
            }
        }
    }

    // NOTE: In case window is minimized, music stream is stopped,
    // just make sure to play again on window restore
    if (IsPlaying())
        Play();
}

void Music::Stop()
{
    stream->Stop();
    inputPlugin.seek(&waveInfo, 0);
}

void Music::Pause()
{
    stream->Pause();
}

void Music::Resume()
{
    stream->Resume();
}

void Music::Seek(double position)
{
    if (position < 0.0)
        position = 0;

    auto positionInFrames = (int64_t)(position * (double)stream->GetSampleRate());

    if (inputPlugin.seek(&waveInfo, positionInFrames))
        stream->buffer->framesProcessed = positionInFrames;
}

void Music::SetVolume(float volume)
{
    stream->SetVolume(volume);
}

void Music::SetPitch(float pitch)
{
    stream->SetPitch(pitch);
}

void Music::SetPan(float pan)
{
    stream->SetPan(pan);
}

double Music::GetTimeLength()
{
    return (double)frameCount / (double)stream->GetSampleRate();
}

double Music::GetTimePlayed()
{
    if (!stream->IsReady())
        return {};

    auto framesProcessed = stream->buffer->framesProcessed;
    auto subBufferSize = stream->buffer->sizeInFrames / 2;
    auto framesInFirstBuffer = stream->buffer->isSubBufferProcessed[0] ? 0 : subBufferSize;
    auto framesInSecondBuffer = stream->buffer->isSubBufferProcessed[1] ? 0 : subBufferSize;
    auto framesSentToMix = stream->buffer->frameCursorPos % subBufferSize;
    auto framesPlayed = (framesProcessed - framesInFirstBuffer - framesInSecondBuffer + framesSentToMix) % frameCount;
    if (framesPlayed < 0)
        framesPlayed += frameCount;

    return (double)framesPlayed / (double)stream->GetSampleRate();
}

bool Music::GetValue(const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept
{
    return inputPlugin.getValue(const_cast<RAudio2_WaveInfo*>(&waveInfo), key, keyLength, valueOut);
}

bool Music::GetArchiveValue(const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept
{
    if (archivePlugin)
        return archivePlugin.getValue(archiveFileCtx, key, keyLength, valueOut);
    return false;
}
