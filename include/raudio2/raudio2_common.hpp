#pragma once

#include "raudio2_common.h"
#include <string>
#include <string_view>

namespace ra
{
    // splits a key into 2 parts (key=input.wav.plugin_extensions returns [input, wav.plugin_extensions])
    template <class T = std::string_view>
    std::pair<T, T> splitKey(const std::string_view key)
    {
        auto pos = key.find('.', 0);
        if (pos != T::npos)
        {
            return { key.substr(0, pos), key.substr(pos + 1, key.size() - pos) };
        }
        return { key, "" };
    }

    template <class T>
    constexpr T str2intT(const char* str, size_t length) noexcept
    {
        T hash = 5381;
        size_t i = 0;
        char c = 0;
        for (; i < length; i++)
        {
            c = str[i];
            hash = ((hash << 5) + hash) + c;
        }
        return hash;
    }

    template <size_t Length>
    constexpr uint32_t str2int(const char (&str)[Length]) noexcept
    {
        return str2intT<uint32_t>(str, Length - 1);
    }

    template <class T = uint32_t>
    T str2int(const ::std::string& str)
    {
        return str2intT<T>(str.data(), str.size());
    }

    template <class T = uint32_t>
    constexpr T str2int(const ::std::string_view str)
    {
        return str2intT<T>(str.data(), str.size());
    }
}
