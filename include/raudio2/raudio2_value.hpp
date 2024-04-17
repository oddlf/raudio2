#pragma once

#include "raudio2_value.h"
#include <array>
#include <string_view>

namespace ra
{
    class Value
    {
    private:
        RAudio2_Value value{};

    public:
        Value() noexcept {}
        virtual ~Value() = default;

        auto getValue() noexcept { return &value; }
        auto getValue() const noexcept { return &value; }

        auto getType() const noexcept { return (RAudio2_ValueType)value.type; }

        explicit operator bool() const noexcept
        {
            return value.type > RAUDIO2_VALUE_NONE &&
                   value.type < RAUDIO2_VALUE_COUNT;
        }

        template <class T>
        T getValue(const std::string_view key) const noexcept
        {
            auto type = RAUDIO2_VALUE_NONE;
            if constexpr (std::is_integral_v<T>)
                type = RAUDIO2_VALUE_INT64;
            else if constexpr (std::is_floating_point_v<T>)
                type = RAUDIO2_VALUE_DOUBLE;

            if (value.type == type)
            {
                if constexpr (std::is_integral_v<T>)
                    return value.value.num;
                else if constexpr (std::is_floating_point_v<T>)
                    return value.value.numf;
            }
            return {};
        }

        bool getBool(const std::string_view key) const noexcept { return getValue<bool>(key); }

        int64_t getInt64(const std::string_view key) const noexcept { return getValue<int64_t>(key); }

        double getDouble(const std::string_view key) const noexcept { return getValue<double>(key); }

        const char* getStringChar(const std::string_view key) const noexcept
        {
            if (value.type == RAUDIO2_VALUE_STRING)
            {
                return value.value.str;
            }
            return {};
        }

        const std::string_view getStringView(const std::string_view key) const noexcept
        {
            if (value.type == RAUDIO2_VALUE_STRING)
            {
                return std::string_view(value.value.str, value.size);
            }
            return {};
        }

        const char** getStringCharArray(const std::string_view key) const noexcept
        {
            if (value.type == RAUDIO2_VALUE_POINTER)
            {
                return (const char**)value.value.ptr;
            }
            return {};
        }
    };

    template <class T>
    bool MakeValue(T inValue, RAudio2_Value& valueOut)
    {
        // std::string_view
        if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, const std::string_view>)
        {
            if (!inValue.empty())
            {
                valueOut.value.str = inValue.data();
                valueOut.size = (uint32_t)inValue.size();
                valueOut.type = RAUDIO2_VALUE_STRING;
                return true;
            }
        }
        // char* string
        else if constexpr (std::is_same_v<T, char*> || std::is_same_v<T, const char*>)
        {
            if (inValue)
            {
                std::string_view str(inValue);
                valueOut.value.str = str.data();
                valueOut.size = (uint32_t)str.size();
                valueOut.type = RAUDIO2_VALUE_STRING;
                return true;
            }
        }
        // bool, int, long, ...
        else if constexpr (std::is_integral_v<T>)
        {
            valueOut.value.num = (int64_t)inValue;
            valueOut.size = sizeof(int64_t);
            valueOut.type = RAUDIO2_VALUE_INT64;
            return true;
        }
        // float, double
        else if constexpr (std::is_floating_point_v<T>)
        {
            valueOut.value.num = (double)inValue;
            valueOut.size = sizeof(double);
            valueOut.type = RAUDIO2_VALUE_DOUBLE;
            return true;
        }
        valueOut.value.ptr = nullptr;
        valueOut.size = 0;
        valueOut.type = RAUDIO2_VALUE_NONE;
        return false;
    }

    template <class T, size_t _Size>
    bool MakeArrayValue(const std::array<T, _Size>& inValue, RAudio2_Value& valueOut)
    {
        valueOut.value.ptr = (void*)inValue.data();
        valueOut.size = (uint32_t)inValue.size() - 1;
        valueOut.type = RAUDIO2_VALUE_POINTER;
        return true;
    }

    template <class T>
    bool MakeArrayValue(const T& inValue, RAudio2_Value& valueOut)
    {
        valueOut.value.ptr = (void*)inValue.data();
        valueOut.size = (uint32_t)inValue.size() - 1;
        valueOut.type = RAUDIO2_VALUE_POINTER;
        return true;
    }
}
