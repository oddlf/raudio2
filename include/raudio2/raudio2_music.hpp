#pragma once

#include "raudio2/raudio2.h"
#include "raudio2/raudio2_value.hpp"
#include <string_view>

namespace ra
{
    using namespace std::literals;

    // Music (calls Unload() on destruction)
    class Music
    {
    protected:
        RAUDIO2_HANDLE raHandle{};
        int32_t id{};

        Music(RAUDIO2_HANDLE handle, int32_t musicId) noexcept : raHandle(handle), id(musicId){};

    public:
        Music() = default;
        virtual ~Music() { Unload(); }

        Music(Music const&) noexcept = delete;
        Music& operator=(Music const&) noexcept = delete;

        Music(Music&& other) noexcept
        {
            Unload();
            raHandle = other.raHandle;
            id = other.id;
            other.raHandle = nullptr;
            other.id = 0;
        }
        Music& operator=(Music&& other) noexcept
        {
            if (this != &other)
            {
                Unload();
                raHandle = other.raHandle;
                id = other.id;
                other.raHandle = nullptr;
                other.id = 0;
            }
            return *this;
        }

        auto Handle() const noexcept { return raHandle; }
        auto Id() const noexcept { return id; }

        explicit operator bool() const noexcept { return IsReady(); }

        bool Load(RAUDIO2_HANDLE handle, const char* fileName, bool streamFile) noexcept
        {
            if (id || !handle)
                return false;

            id = RAudio2_LoadMusic(handle, fileName, streamFile);
            if (id != 0)
            {
                raHandle = handle;
                return true;
            }
            raHandle = {};
            return false;
        }
        bool Load(RAUDIO2_HANDLE handle, const char* fileType, const unsigned char* data, int64_t dataSize) noexcept
        {
            if (id || !handle)
                return false;

            id = RAudio2_LoadMusicFromMemory(handle, fileType, data, dataSize);
            if (id != 0)
            {
                raHandle = handle;
                return true;
            }
            raHandle = {};
            return false;
        }
        void Unload() noexcept
        {
            if (id)
            {
                RAudio2_UnloadMusic(raHandle, id);
                id = {};
                raHandle = {};
            }
        }
        bool IsReady() const noexcept { return RAudio2_IsMusicReady(raHandle, id); }
        auto Play() const noexcept { RAudio2_PlayMusic(raHandle, id); }
        auto IsPlaying() const noexcept { return RAudio2_IsMusicPlaying(raHandle, id); }
        auto IsStopped() const noexcept { return RAudio2_IsMusicStopped(raHandle, id); }
        auto Update() const noexcept { RAudio2_UpdateMusic(raHandle, id); }
        auto Stop() const noexcept { RAudio2_StopMusic(raHandle, id); }
        auto Pause() const noexcept { RAudio2_PauseMusic(raHandle, id); }
        auto Resume() const noexcept { RAudio2_ResumeMusic(raHandle, id); }
        auto Seek(double position) const noexcept { RAudio2_SeekMusic(raHandle, id, position); }
        auto SetVolume(float volume) const noexcept { RAudio2_SetMusicVolume(raHandle, id, volume); }
        auto SetPitch(float pitch) const noexcept { RAudio2_SetMusicPitch(raHandle, id, pitch); }
        auto SetPan(float pan) const noexcept { RAudio2_SetMusicPan(raHandle, id, pan); }
        auto GetTimeLength() const noexcept { return RAudio2_GetMusicTimeLength(raHandle, id); }
        auto GetTimePlayed() const noexcept { return RAudio2_GetMusicTimePlayed(raHandle, id); }

        bool getBool(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getBool(key);
            }
            return {};
        }

        int64_t getInt64(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getInt64(key);
            }
            return {};
        }

        double getDouble(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getDouble(key);
            }
            return {};
        }

        const char* getStringChar(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getStringChar(key);
            }
            return {};
        }

        const std::string_view getStringView(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getStringView(key);
            }
            return {};
        }

        const char** getStringCharArray(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getStringCharArray(key);
            }
            return {};
        }

        bool getArchiveBool(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicArchiveValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getBool(key);
            }
            return {};
        }

        int64_t getArchiveInt64(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicArchiveValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getInt64(key);
            }
            return {};
        }

        double getArchiveDouble(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicArchiveValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getDouble(key);
            }
            return {};
        }

        const char* getArchiveStringChar(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicArchiveValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getStringChar(key);
            }
            return {};
        }

        const std::string_view getArchiveStringView(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicArchiveValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getStringView(key);
            }
            return {};
        }

        const char** getArchiveStringCharArray(const std::string_view key) const noexcept
        {
            Value value;
            if (RAudio2_GetMusicArchiveValue(raHandle, id, key.data(), (uint32_t)key.size(), value.getValue()))
            {
                return value.getStringCharArray(key);
            }
            return {};
        }

        auto getArchivePluginName() const noexcept { return getArchiveStringView("plugin_name"sv); }

        auto getInputPluginName() const noexcept { return getStringView("plugin_name"sv); }

        bool isVBR() const noexcept { return getBool("vbr"sv); }

        int64_t getBps() const noexcept { return getInt64("bps"sv); }

        double getKbps() const noexcept { return (double)getBps() / 1000.0; }

        int64_t getCurrentBps() const noexcept { return getInt64("current_bps"sv); }

        double getCurrentKbps() const noexcept { return (double)getCurrentBps() / 1000.0; }

        auto getFormat() const noexcept { return getStringView("format"sv); }

        auto getAlbum() const noexcept { return getStringView("album"sv); }

        auto getArtist() const noexcept { return getStringView("artist"sv); }

        auto getComment() const noexcept { return getStringView("comment"sv); }

        auto getYear() const noexcept { return getStringView("year"sv); }

        auto getTitle() const noexcept { return getStringView("title"sv); }
    };

    // Non owning Music (doesn't call Unload() on destruction)
    class MusicView : public Music
    {
    public:
        MusicView(Music& music) noexcept : MusicView(music.Handle(), music.Id()){};
        MusicView(RAUDIO2_HANDLE handle, int32_t musicId) noexcept : Music(handle, musicId){};
        ~MusicView() noexcept
        {
            raHandle = {};
            id = {};
        }

        MusicView(MusicView const&) noexcept = default;
        MusicView& operator=(MusicView const&) noexcept = default;

        MusicView(MusicView&& other) noexcept = default;
        MusicView& operator=(MusicView&& other) noexcept = default;
    };
}
