#pragma once

#include "raudio2_inputplugin.h"
#include "raudio2_value.hpp"
#include <string_view>

namespace ra
{
    using namespace std::literals;

    class InputPlugin
    {
    private:
        const RAudio2_InputPlugin* plugin{};

    public:
        InputPlugin() noexcept {}
        InputPlugin(const RAudio2_InputPlugin& plugin_) noexcept : plugin(&plugin_) {}
        InputPlugin(const RAudio2_InputPlugin* plugin_) noexcept : plugin(plugin_) {}
        virtual ~InputPlugin() = default;

        explicit operator bool() const noexcept { return plugin != nullptr; }

        bool hasValidOpen() const noexcept { return plugin->open != nullptr; }
        bool hasValidRead() const noexcept { return plugin->read != nullptr; }
        bool hasValidSeek() const noexcept { return plugin->seek != nullptr; }
        bool hasValidClose() const noexcept { return plugin->close != nullptr; }
        bool hasValidGetValue() const noexcept { return plugin->getValue != nullptr; }

        bool open(RAudio2_WaveInfo* wave) const noexcept { return plugin->open(wave); }

        auto read(RAudio2_WaveInfo* wave, void* bufferOut, int64_t framesToRead) const noexcept { return plugin->read(wave, bufferOut, framesToRead); }

        bool seek(RAudio2_WaveInfo* wave, int64_t positionInFrames) const noexcept { return plugin->seek(wave, positionInFrames); }

        bool close(RAudio2_WaveInfo* wave) const noexcept { return plugin->close(wave); }

        bool getValue(RAudio2_WaveInfo* wave, const char* key, int32_t keyLength, RAudio2_Value* valueOut) const noexcept
        {
            return plugin->getValue(wave, key, keyLength, valueOut);
        }

        bool getValue(RAudio2_WaveInfo* wave, const std::string_view key, RAudio2_Value* valueOut) const noexcept
        {
            return plugin->getValue(wave, key.data(), (int32_t)key.size(), valueOut);
        }

        bool getValue(const std::string_view key, RAudio2_Value* valueOut) const noexcept
        {
            return plugin->getValue(nullptr, key.data(), (int32_t)key.size(), valueOut);
        }

        bool getBool(const std::string_view key) const noexcept
        {
            return getBool(nullptr, key);
        }

        bool getBool(RAudio2_WaveInfo* wave, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(wave, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getBool(key);
            }
            return {};
        }

        int64_t getInt64(const std::string_view key) const noexcept
        {
            return getInt64(nullptr, key);
        }

        int64_t getInt64(RAudio2_WaveInfo* wave, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(wave, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getInt64(key);
            }
            return {};
        }

        double getDouble(const std::string_view key) const noexcept
        {
            return getDouble(nullptr, key);
        }

        double getDouble(RAudio2_WaveInfo* wave, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(wave, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getDouble(key);
            }
            return {};
        }

        const char* getStringChar(const std::string_view key) const noexcept
        {
            return getStringChar(nullptr, key);
        }

        const char* getStringChar(RAudio2_WaveInfo* wave, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(wave, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringChar(key);
            }
            return {};
        }

        const std::string_view getStringView(const std::string_view key) const noexcept
        {
            return getStringView(nullptr, key);
        }

        const std::string_view getStringView(RAudio2_WaveInfo* wave, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(wave, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringView(key);
            }
            return {};
        }

        const char** getStringCharArray(const std::string_view key) const noexcept
        {
            return getStringCharArray(nullptr, key);
        }

        const char** getStringCharArray(RAudio2_WaveInfo* wave, const std::string_view key) const noexcept
        {
            Value value;
            if (getValue(wave, key.data(), (int32_t)key.size(), value.getValue()))
            {
                return value.getStringCharArray(key);
            }
            return {};
        }

        const std::string_view getName() const noexcept { return getStringView("plugin_name"sv); }

        const char** getExtensions() const noexcept { return getStringCharArray("plugin_extensions"sv); }
    };
}
