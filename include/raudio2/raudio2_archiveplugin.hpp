#pragma once

#include "raudio2_archiveplugin.h"
#include "raudio2_value.hpp"
#include <string_view>

namespace ra
{
    using namespace std::literals;

    class ArchivePlugin
    {
    private:
        const RAudio2_ArchivePlugin* plugin{};

    public:
        ArchivePlugin() noexcept {}
        ArchivePlugin(const RAudio2_ArchivePlugin& plugin_) noexcept : plugin(&plugin_) {}
        ArchivePlugin(const RAudio2_ArchivePlugin* plugin_) noexcept : plugin(plugin_) {}
        virtual ~ArchivePlugin() = default;

        explicit operator bool() const noexcept { return plugin != nullptr; }

        bool hasValidArchiveOpen() const noexcept { return plugin->archiveOpen != nullptr; }
        bool hasValidArchiveClose() const noexcept { return plugin->archiveClose != nullptr; }
        bool hasValidFileOpen() const noexcept { return plugin->fileOpen != nullptr; }
        bool hasValidFileRead() const noexcept { return plugin->fileRead != nullptr; }
        bool hasValidFileSeek() const noexcept { return plugin->fileSeek != nullptr; }
        bool hasValidFileTell() const noexcept { return plugin->fileTell != nullptr; }
        bool hasValidFileGetSize() const noexcept { return plugin->fileGetSize != nullptr; }
        bool hasValidFileClose() const noexcept { return plugin->fileClose != nullptr; }
        bool hasValidGetValue() const noexcept { return plugin->getValue != nullptr; }

        bool archiveOpen(RAudio2_Archive* archive) const noexcept { return plugin->archiveOpen(archive); }

        bool archiveClose(RAudio2_Archive* archive) const noexcept { return plugin->archiveClose(archive); }

        bool fileOpen(RAudio2_Archive* archive, const char* filePath, void** fileCtxOut) const noexcept { return plugin->fileOpen(archive, filePath, fileCtxOut); }

        int64_t fileRead(void* fileCtx, void* bufferOut, int64_t bytesToRead) const noexcept { return plugin->fileRead(fileCtx, bufferOut, bytesToRead); }

        int64_t fileSeek(void* fileCtx, int64_t offset, int whence) const noexcept { return plugin->fileSeek(fileCtx, offset, whence); }

        int64_t fileTell(void* fileCtx) const noexcept { return plugin->fileTell(fileCtx); }

        int64_t fileGetSize(void* fileCtx) const noexcept { return plugin->fileGetSize(fileCtx); }

        bool fileClose(void* fileCtx) const noexcept { return plugin->fileClose(fileCtx); }

        bool getValue(void* fileCtx, const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept
        {
            return plugin->getValue(fileCtx, key, keyLength, valueOut);
        }

        bool getValue(void* fileCtx, const std::string_view key, RAudio2_Value* valueOut) const noexcept
        {
            return plugin->getValue(fileCtx, key.data(), (int32_t)key.size(), valueOut);
        }

        bool getValue(const std::string_view key, RAudio2_Value* valueOut) const noexcept
        {
            return plugin->getValue(nullptr, key.data(), (int32_t)key.size(), valueOut);
        }

        bool getBool(const std::string_view key) const noexcept
        {
            return getBool(nullptr, key);
        }

        bool getBool(void* fileCtx, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(fileCtx, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getBool(key);
            }
            return {};
        }

        int64_t getInt64(const std::string_view key) const noexcept
        {
            return getInt64(nullptr, key);
        }

        int64_t getInt64(void* fileCtx, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(fileCtx, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getInt64(key);
            }
            return {};
        }

        double getDouble(const std::string_view key) const noexcept
        {
            return getDouble(nullptr, key);
        }

        double getDouble(void* fileCtx, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(fileCtx, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getDouble(key);
            }
            return {};
        }

        const char* getStringChar(const std::string_view key) const noexcept
        {
            return getStringChar(nullptr, key);
        }

        const char* getStringChar(void* fileCtx, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(fileCtx, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringChar(key);
            }
            return {};
        }

        const std::string_view getStringView(const std::string_view key) const noexcept
        {
            return getStringView(nullptr, key);
        }

        const std::string_view getStringView(void* fileCtx, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(fileCtx, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringView(key);
            }
            return {};
        }

        const char** getStringCharArray(const std::string_view key) const noexcept
        {
            return getStringCharArray(nullptr, key);
        }

        const char** getStringCharArray(void* fileCtx, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(fileCtx, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringCharArray(key);
            }
            return {};
        }

        const std::string_view getName() const noexcept { return getStringView("plugin_name"sv); }

        const char** getExtensions() const noexcept { return getStringCharArray("plugin_extensions"sv); }
    };
}
